// In the case of a timeout, we attempt to recover by manually clocking SCL. The
// idea is that if the bus is locked up, a slave is likely holding SDA low in
// the middle of a transaction. By clocking SCL, we hopefully complete the
// slave's transaction and transition it into an idle state for the next valid
// transaction.
#include "i2c.h"

#include <stdbool.h>

#include "log.h"
#include "queues.h"
#include "semaphore.h"
#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_interrupt.h"

typedef enum I2CMode {
  I2C_MODE_TRANSMIT = 0,
  I2C_MODE_RECEIVE,
  NUM_I2C_MODES,
} I2CMode;

// Used to protect I2C resources
// All data tx'd and rx'd through queue
// Transactions on each I2C port protected by Mutex
typedef struct {
  uint8_t buf[I2C_MAX_NUM_DATA];
  Queue queue;
  Semaphore mutex;
} I2CBuffer;

typedef struct {
  uint32_t periph;                // Clock config
  uint8_t ev_irqn;                // Event Interrupt
  uint8_t err_irqn;               // Error Interrupt
  I2C_TypeDef *base;              // Base Peripheral defined by CMSIS
  I2CSettings settings;           // I2C Config
  I2CBuffer i2c_buf;              // RTOS data structures
  bool curr_mode;                 // Mode used in current transxn
  I2CAddress current_addr;        // Addr used in current transxn
  volatile uint8_t num_rx_bytes;  // Number of bytes left to receive in rx mode
} I2CPortData;

static I2CPortData s_port[NUM_I2C_PORTS] = {
  [I2C_PORT_1] = { .periph = RCC_APB1Periph_I2C1,
                   .base = I2C1,
                   .ev_irqn = I2C1_EV_IRQn,
                   .err_irqn = I2C1_ER_IRQn },
  [I2C_PORT_2] = { .periph = RCC_APB1Periph_I2C2,
                   .base = I2C2,
                   .ev_irqn = I2C2_EV_IRQn,
                   .err_irqn = I2C2_ER_IRQn },
};

// Generated using the I2C timing configuration tool (STSW-STM32126)
// static const uint32_t s_i2c_timing[] = {
//   [I2C_SPEED_STANDARD] = 0x10805E89,  // 100 kHz
//   [I2C_SPEED_FAST] = 0x00901850,      // 400 kHz
// };
static const uint32_t s_i2c_timing[] = {
  [I2C_SPEED_STANDARD] = 100000,  // 100 kHz
  [I2C_SPEED_FAST] = 400000,      // 400 kHz
};

// static void prv_recover_lockup(I2CPort port) {
//   I2C_DeInit(port);
//   I2CSettings *settings = &s_port[port].settings;
// 
//   // Manually clock SCL
//   gpio_init_pin(&settings->scl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
//   for (size_t i = 0; i < 16; i++) {
//     gpio_toggle_state(&settings->scl);
//   }
// 
//   gpio_init_pin(&settings->scl, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_HIGH);
// 
//   // Reset I2C
//   I2C_SoftwareResetCmd(s_port[port].base, ENABLE);
//   I2C_SoftwareResetCmd(s_port[port].base, DISABLE);
// }

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  } else if (settings->speed >= NUM_I2C_SPEEDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C speed.");
  }

  s_port[i2c].settings = *settings;

  // Enable clock for I2C
  RCC_APB1PeriphClockCmd(s_port[i2c].periph, ENABLE);

  // Enable GPIOB clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  // Remap pins to I2C pins 8 & 9 on Port 1
  if(i2c == I2C_PORT_1) {
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
  }

  // Initialize pins to correct mode to operate I2C
  gpio_init_pin(&(settings->scl), GPIO_ALTFN_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&(settings->sda), GPIO_ALFTN_OPEN_DRAIN, GPIO_STATE_HIGH);

  // Initialize I2C peripheral with settings
  I2C_InitTypeDef i2c_init;
  I2C_StructInit(&i2c_init);
  i2c_init.I2C_Mode = I2C_Mode_I2C;
  i2c_init.I2C_Ack = I2C_Ack_Enable;
  i2c_init.I2C_ClockSpeed = s_i2c_timing[settings->speed];
  I2C_Init(s_port[i2c].base, &i2c_init);

  // Enable global interrupts
  stm32f10x_interrupt_nvic_enable(s_port[i2c].ev_irqn, INTERRUPT_PRIORITY_NORMAL);
  stm32f10x_interrupt_nvic_enable(s_port[i2c].err_irqn, INTERRUPT_PRIORITY_NORMAL);
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_SetPriority(I2C1_EV_IRQn, 2);


  // Mask interrupts while we are not in a transaction
  // This prevents TXE from continuously triggering
  I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT | I2C_IT_BUF, DISABLE);

  s_port[i2c].i2c_buf.queue.num_items = I2C_MAX_NUM_DATA;
  s_port[i2c].i2c_buf.queue.item_size = sizeof(uint8_t);
  s_port[i2c].i2c_buf.queue.storage_buf = s_port[i2c].i2c_buf.buf;
  status_ok_or_return(sem_init(&s_port[i2c].i2c_buf.mutex, 1, 0));
  status_ok_or_return(queue_init(&s_port[i2c].i2c_buf.queue));

  // Enable I2C peripheral
  I2C_Cmd(s_port[i2c].base, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  // Lock I2C resource
  // status_ok_or_return(sem_wait(&s_port[i2c].i2c_buf.mutex, 5 * I2C_TIMEOUT_MS));

  // Check that bus is not busy - If it is, assume that lockup has occurred
  // if (I2C_GetFlagStatus(s_port[i2c].base, I2C_FLAG_BUSY) == SET) {
  //   prv_recover_lockup(i2c);
  // }

  // Set number of bytes to read
  s_port[i2c].num_rx_bytes = rx_len;
  // Store address for this transaction
  s_port[i2c].current_addr = addr;
  s_port[i2c].curr_mode = I2C_MODE_RECEIVE;

  // Enable acknowlege, as it is disabled at end of every rx transaction
  I2C_AcknowledgeConfig(s_port[i2c].base, ENABLE);

  // Enable Interrupts before setting start
  I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT, ENABLE);

  // Start Transaction
  I2C_GenerateSTART(s_port[i2c].base, ENABLE);

  // Wait for mutex to be unlocked from ISR
  // If we timeout, it means some issue has occurred, so we will dump queue
  if (sem_wait(&s_port[i2c].i2c_buf.mutex, I2C_TIMEOUT_MS)) {
    I2C_GenerateSTOP(s_port[i2c].base, ENABLE);
    I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT, DISABLE);
    queue_reset(&s_port[i2c].i2c_buf.queue);
    sem_post(&s_port[i2c].i2c_buf.mutex);
    return STATUS_CODE_TIMEOUT;
  }

  // Receive data from queue
  // If less than requested is received, an error in the transaction has occurred
  for (size_t rx = 0; rx < rx_len; rx++) {
    if (queue_receive(&s_port[i2c].i2c_buf.queue, &rx_data[rx], 0)) {
      sem_post(&s_port[i2c].i2c_buf.mutex);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  sem_post(&s_port[i2c].i2c_buf.mutex);
  return STATUS_CODE_OK;
}

// Address needs to be just the device address, read/write bit is taken care of in hardware
StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  // status_ok_or_return(sem_wait(&s_port[i2c].i2c_buf.mutex, I2C_TIMEOUT_MS));
  // Check that bus is not busy - If it is, assume that lockup has occurred
  // if (I2C_GetFlagStatus(s_port[i2c].base, I2C_FLAG_BUSY) == SET) {
  //   prv_recover_lockup(i2c);
  // }

  // Copy data into queue
  for (size_t tx = 0; tx < tx_len; tx++) {
    if (queue_send(&s_port[i2c].i2c_buf.queue, &tx_data[tx], 0)) {
      queue_reset(&s_port[i2c].i2c_buf.queue);
      sem_post(&s_port[i2c].i2c_buf.mutex);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  // Store address for this transaction
  s_port[i2c].current_addr = (addr) << 1;
  s_port[i2c].curr_mode = I2C_MODE_TRANSMIT;

  // Start an I2C transaction by enabling start bit. Transfers occur in IT handler
  // Start bit is cleared when I2C_GetITStatus() and I2C_SendData() called in succession
  I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT, ENABLE);
  // I2C1->CR2 |= I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN;
  I2C_GenerateSTART(s_port[i2c].base, ENABLE);

  // Wait for ISR to unlock mutex when transaction finished
  // If we timeout, it means some issue has occurred, so we will dump queue
  if (sem_wait(&s_port[i2c].i2c_buf.mutex, I2C_TIMEOUT_MS)) {
    I2C_GenerateSTOP(s_port[i2c].base, ENABLE);
    I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT, DISABLE);
    queue_reset(&s_port[i2c].i2c_buf.queue);
    sem_post(&s_port[i2c].i2c_buf.mutex);
    return STATUS_CODE_TIMEOUT;
  }
  sem_post(&s_port[i2c].i2c_buf.mutex);
  return STATUS_CODE_OK;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                        size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }

  uint8_t reg_to_read = reg;
  status_ok_or_return(i2c_write(i2c, addr, &reg_to_read, sizeof(reg)));
  status_ok_or_return(i2c_read(i2c, addr, rx_data, rx_len));
  return STATUS_CODE_OK;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }

  uint8_t reg_to_write = reg;
  status_ok_or_return(i2c_write(i2c, addr, &reg_to_write, sizeof(reg)));
  status_ok_or_return(i2c_write(i2c, addr, tx_data, tx_len));
  return STATUS_CODE_OK;
}

// IRQ functionality for the I2C event interruptd
// Since activity is the same for both ports, use one IRQ handler
// Flags are cleared automatically by respective reads and writes
static void prv_ev_irq_handler(I2CPort i2c) {
  BaseType_t xTaskWoken = pdFALSE;

  // Second event is I2C_IT_ADDR being set once address is sent
  if (I2C_GetITStatus(s_port[i2c].base, I2C_IT_ADDR) == SET) {
    // Reading SR2 register clears ADDR IT flag
    (void)(s_port[i2c].base->SR2);

    // If we only have one byte to receive, need to generate stop condition now
    if (s_port[i2c].curr_mode == I2C_MODE_RECEIVE && s_port[i2c].num_rx_bytes == 1) {
      I2C_AcknowledgeConfig(s_port[i2c].base, DISABLE);
      I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT, DISABLE);
      I2C_GenerateSTOP(s_port[i2c].base, ENABLE);
      xSemaphoreGiveFromISR(s_port[i2c].i2c_buf.mutex.handle, &xTaskWoken);
    }
  }

  // First event triggered when start bit is sent
  if (I2C_GetITStatus(s_port[i2c].base, I2C_IT_SB) == SET) {
    // Send address, with LSB set for Read, reset for Write
    // Reading IT status and writing Data Reg clears Start bit
    I2C_Send7bitAddress(s_port[i2c].base, s_port[i2c].current_addr, s_port[i2c].curr_mode);

  // In write (tx) mode, send a byte whenever TX register is empty
  } else if (I2C_GetITStatus(s_port[i2c].base, I2C_IT_TXE)) {
    uint8_t tx_data = 0;
    if (xQueueReceiveFromISR(s_port[i2c].i2c_buf.queue.handle, &tx_data, &xTaskWoken)) {
      I2C_SendData(s_port[i2c].base, tx_data);
    } else {  // If we have sent all data, stop transaction and signal task
      I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT, DISABLE);
      I2C_GenerateSTOP(s_port[i2c].base, ENABLE);
      xSemaphoreGiveFromISR(s_port[i2c].i2c_buf.mutex.handle, &xTaskWoken);
    }

    // In read (rx) mode, read value from data in reg, send to queue
  } else if (I2C_GetITStatus(s_port[i2c].base, I2C_IT_RXNE)) {
    uint8_t rx_data = I2C_ReceiveData(s_port[i2c].base);
    xQueueSendFromISR(s_port[i2c].i2c_buf.queue.handle, &rx_data, &xTaskWoken);

    s_port[i2c].num_rx_bytes--;
    if (s_port[i2c].num_rx_bytes == 1) {
      // If we only have one byte left, don't ack slave
      // This finishes the transaction after last byte sent
      I2C_AcknowledgeConfig(s_port[i2c].base, DISABLE);
      I2C_ITConfig(s_port[i2c].base, I2C_IT_ERR | I2C_IT_EVT, DISABLE);
      I2C_GenerateSTOP(s_port[i2c].base, ENABLE);
    } else if (s_port[i2c].num_rx_bytes == 0) {
      // Unlock mutex after receiving last byte
      xSemaphoreGiveFromISR(s_port[i2c].i2c_buf.mutex.handle, &xTaskWoken);
    }
  }
  portYIELD_FROM_ISR(xTaskWoken);
}

static void prv_dump_queue_from_isr(I2CPort i2c) {
  uint8_t rx_data;
  while (!xQueueIsQueueEmptyFromISR(s_port[i2c].i2c_buf.queue.handle)) {
    xQueueReceiveFromISR(s_port[i2c].i2c_buf.queue.handle, &rx_data, NULL);
  }
}

static void prv_err_irq_handler(I2CPort i2c) {
  // Bus error
  if (I2C_GetITStatus(s_port[i2c].base, I2C_IT_BERR) == SET) {
    // if bus error has occurred, dump queue load. This will be caught by
    // tx/rx method when it tries to read full data length
    prv_dump_queue_from_isr(i2c);
    I2C_ClearITPendingBit(s_port[i2c].base, I2C_IT_BERR);

    // Slave Acknowledge failure
  } else if (I2C_GetITStatus(s_port[i2c].base, I2C_IT_AF) == SET) {
    // Dump queue, so transaction will register as failed
    prv_dump_queue_from_isr(i2c);
    I2C_ClearITPendingBit(s_port[i2c].base, I2C_IT_AF);

  } else {
    // This should not happen since we do not use multi-master.
    // Simply clear all other error flags
    I2C_ClearITPendingBit(s_port[i2c].base, I2C_IT_ARLO | I2C_IT_OVR | I2C_IT_TIMEOUT |
                                                I2C_IT_PECERR | I2C_IT_SMBALERT);
  }
}

void I2C1_EV_IRQHandler() {
  prv_ev_irq_handler(I2C_PORT_1);
}

void I2C2_EV_IRQHandler() {
  prv_ev_irq_handler(I2C_PORT_2);
}

void I2C1_ER_IRQHandler() {
  prv_err_irq_handler(I2C_PORT_1);
}
void I2C2_ER_IRQHandler() {
  prv_err_irq_handler(I2C_PORT_2);
}
