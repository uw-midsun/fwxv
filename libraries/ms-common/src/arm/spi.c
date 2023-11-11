#include "spi.h"

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "queues.h"
#include "semaphore.h"
#include "stm32f10x_interrupt.h"
#include "stm32f10x_spi.h"
#define FLIP_RXNE 0x80
#define FLIP_ERR 0x40

typedef struct {
  uint8_t tx_buf[SPI_MAX_NUM_DATA];
  Queue tx_queue;
  uint8_t rx_buf[SPI_MAX_NUM_DATA];
  Queue rx_queue;
  Semaphore mutex;
} SPIBuffer;

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  uint8_t irqn;
  SpiMode mode;
  SPI_TypeDef *base;
  GpioAddress cs;
  SPIBuffer spi_buf;
  volatile uint8_t num_rx_bytes;  // Number of bytes left to receive in rx mode
} SpiPortData;

static SpiPortData s_port[NUM_SPI_PORTS] = {
  [SPI_PORT_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                   .periph = RCC_APB2Periph_SPI1,
                   .base = SPI1,
                   .irqn = SPI1_IRQn },
  [SPI_PORT_2] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                   .periph = RCC_APB1Periph_SPI2,
                   .base = SPI2,
                   .irqn = SPI2_IRQn },
};

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  } else if (settings->mode >= NUM_SPI_MODES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI mode.");
  }

  s_port[spi].rcc_cmd(s_port[spi].periph, ENABLE);
  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);
  uint32_t clk_freq;

  if (spi == SPI_PORT_1) {
    clk_freq = clocks.PCLK2_Frequency;  // SPI1 is on APB2 Bus
  } else {
    clk_freq = clocks.PCLK1_Frequency;  // SPI2 is on APB1 Bus
  }

  // SPI prescalers must be powers of two with a minimum prescaler of /2,
  // The equation is baudrate = Frequency/Prescaler, rounded to nearest power of 2
  // This is then offset to a valid SPI_BaudRate_Prescaler
  size_t index = 32 - (size_t)__builtin_clz(clk_freq / settings->baudrate);
  uint16_t prescaler = (index - 2) << 3;
  if (!IS_SPI_BAUDRATE_PRESCALER(prescaler)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid baudrate");
  }
  s_port[spi].cs = settings->cs;

  // Confifgure spi pins to correct modes
  gpio_init_pin(&settings->miso, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&settings->mosi, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  // TODO(mitchellostler): CRC is not used, could be a potential improvement on reliability
  SPI_InitTypeDef init = {
    .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
    .SPI_Mode = SPI_Mode_Master,
    .SPI_DataSize = SPI_DataSize_8b,
    .SPI_CPHA = (settings->mode & 0x01) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge,
    .SPI_CPOL = (settings->mode & 0x02) ? SPI_CPOL_High : SPI_CPOL_Low,
    .SPI_NSS = SPI_NSS_Soft,
    .SPI_BaudRatePrescaler = (index - 2) << 3,
    .SPI_FirstBit = SPI_FirstBit_MSB,
    .SPI_CRCPolynomial = 7,
  };
  SPI_Init(s_port[spi].base, &init);

  // Setup SPI interrupts, error at a higher priority
  stm32f10x_interrupt_nvic_enable(s_port[spi].irqn, INTERRUPT_PRIORITY_NORMAL);

  // Setup queues
  s_port[spi].spi_buf.rx_queue.num_items = SPI_MAX_NUM_DATA;
  s_port[spi].spi_buf.rx_queue.item_size = sizeof(uint8_t);
  s_port[spi].spi_buf.rx_queue.storage_buf = s_port[spi].spi_buf.rx_buf;
  s_port[spi].spi_buf.tx_queue.num_items = SPI_MAX_NUM_DATA;
  s_port[spi].spi_buf.tx_queue.item_size = sizeof(uint8_t);
  s_port[spi].spi_buf.tx_queue.storage_buf = s_port[spi].spi_buf.tx_buf;
  status_ok_or_return(sem_init(&s_port[spi].spi_buf.mutex, 1, 1));
  status_ok_or_return(queue_init(&s_port[spi].spi_buf.rx_queue));
  status_ok_or_return(queue_init(&s_port[spi].spi_buf.tx_queue));

  // Disable interrupts when not in a transaction
  SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE | SPI_I2S_IT_ERR, DISABLE);
  return STATUS_CODE_OK;
}

StatusCode spi_tx(SpiPort spi, uint8_t *tx_data, size_t tx_len) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_EMPTY, "Invalid SPI port.");
  }
  // Proceed if mutex is unlocked
  status_ok_or_return(sem_wait(&s_port[spi].spi_buf.mutex, SPI_TIMEOUT_MS));

  // Copy data into queue
  for (size_t tx = 0; tx < tx_len; tx++) {
    if (queue_send(&s_port[spi].spi_buf.tx_queue, &tx_data[tx], 0)) {
      queue_reset(&s_port[spi].spi_buf.tx_queue);
      sem_post(&s_port[spi].spi_buf.mutex);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  // Enable interrupts and start transaction
  // Transaction started when we spi is enabled, and tx register is empty
  SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_ERR | SPI_I2S_IT_TXE, ENABLE);
  SPI_Cmd(s_port[spi].base, ENABLE);

  // Wait on IT handler to return mutex when txn complete
  if (sem_wait(&s_port[spi].spi_buf.mutex, SPI_TIMEOUT_MS)) {
    SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_ERR | SPI_I2S_IT_TXE, DISABLE);
    // if we time out dump queue
    SPI_Cmd(s_port[spi].base, DISABLE);
    queue_reset(&s_port[spi].spi_buf.tx_queue);
    sem_post(&s_port[spi].spi_buf.mutex);
    return STATUS_CODE_TIMEOUT;
  }
  sem_post(&s_port[spi].spi_buf.mutex);
  return STATUS_CODE_OK;
}

StatusCode spi_rx(SpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  // Proceed if mutex is unlocked
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_EMPTY, "Invalid SPI port.");
  }

  status_ok_or_return(sem_wait(&s_port[spi].spi_buf.mutex, SPI_TIMEOUT_MS));
  s_port[spi].num_rx_bytes = rx_len;

  for (size_t tx = 0; tx < rx_len; tx++) {
    queue_send(&s_port[spi].spi_buf.tx_queue, &placeholder, 0);
    // if () {
    // queue_reset(&s_port[spi].spi_buf.tx_queue);
    // sem_post(&s_port[spi].spi_buf.mutex);
    // return STATUS_CODE_RESOURCE_EXHAUSTED;
    // }
  }
  // Enable interrupts and start transaction
  // Transaction started when we sent first byte, then it handler takes over
  // Seems to potentially be a bug in the SPL where it doesn't flip the bits it's supposed to
  // Hardcode this for now
  // Interrupt for error and then for RXNE
  SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_ERR | SPI_I2S_IT_RXNE, ENABLE);
  s_port[spi].base->CR2 ^= FLIP_RXNE;
  s_port[spi].base->CR2 |= FLIP_ERR;
  SPI_Cmd(s_port[spi].base, ENABLE);

  // Wait on IT handler to return mutex when txn complete
  if (sem_wait(&s_port[spi].spi_buf.mutex, SPI_TIMEOUT_MS)) {
    // if we time out dump queue - txn invalid
    SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_ERR | SPI_I2S_IT_RXNE, DISABLE);
    s_port[spi].base->CR2 ^= FLIP_ERR;
    SPI_Cmd(s_port[spi].base, DISABLE);
    queue_reset(&s_port[spi].spi_buf.rx_queue);
    sem_post(&s_port[spi].spi_buf.mutex);
    return STATUS_CODE_TIMEOUT;
  }
  sem_post(&s_port[spi].spi_buf.mutex);
  xQueueReceive(s_port[spi].spi_buf.rx_queue.handle, rx_data, portMAX_DELAY);
  return STATUS_CODE_OK;
}

StatusCode spi_cs_set_state(SpiPort spi, GpioState state) {
  return gpio_set_state(&s_port[spi].cs, state);
}

StatusCode spi_cs_get_state(SpiPort spi, GpioState *input_state) {
  return gpio_get_state(&s_port[spi].cs, input_state);
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                        size_t rx_len) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_EMPTY, "Invalid SPI port.");
  }
  // Proceed if mutex is unlocked
  status_ok_or_return(sem_wait(&s_port[spi].spi_buf.mutex, SPI_TIMEOUT_MS));

  queue_reset(&s_port[spi].spi_buf.tx_queue);
  queue_reset(&s_port[spi].spi_buf.rx_queue);

  // Copy data into queue
  for (size_t tx = 0; tx < tx_len; tx++) {
    if (queue_send(&s_port[spi].spi_buf.tx_queue, &tx_data[tx], 0)) {
      queue_reset(&s_port[spi].spi_buf.tx_queue);
      sem_post(&s_port[spi].spi_buf.mutex);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }
  uint8_t placeholder = 0;
  for (size_t rx = 0; rx < rx_len; rx++) {
    if (queue_send(&s_port[spi].spi_buf.tx_queue, &placeholder, 0)) {
      queue_reset(&s_port[spi].spi_buf.tx_queue);
      sem_post(&s_port[spi].spi_buf.mutex);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  // Enable interrupts and start transaction
  // Transaction started when we spi is enabled, and tx register is empty
  SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_ERR | SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE, ENABLE);
  // s_port[spi].base->CR2 ^= FLIP_RXNE;
  s_port[spi].base->CR2 |= FLIP_ERR;
  SPI_Cmd(s_port[spi].base, ENABLE);

  // Wait on IT handler to return mutex when txn complete
  StatusCode complete = sem_wait(&s_port[spi].spi_buf.mutex, SPI_TIMEOUT_MS);
  SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_ERR | SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE, DISABLE);
  s_port[spi].base->CR2 ^= FLIP_ERR;
  SPI_Cmd(s_port[spi].base, DISABLE);

  for (size_t tx = 0; tx < tx_len; tx++) {
    queue_receive(&s_port[spi].spi_buf.rx_queue, &placeholder, 0);
  }
  for (size_t rx = 0; rx < rx_len; rx++) {
    queue_receive(&s_port[spi].spi_buf.rx_queue, &rx_data[rx], 0);
  }

  sem_post(&s_port[spi].spi_buf.mutex);
  return complete;
}

static void prv_spi_irq_handler(SpiPort spi) {
  BaseType_t xTaskWoken = pdFALSE;
  if (SPI_I2S_GetITStatus(s_port[spi].base, SPI_I2S_IT_TXE) == SET) {
    // Handle TX
    // Receive byte in top byte of 16-bit reg
    // Since we are configured in 8bit mode spi should know to send just the one byte
    uint16_t tx_data;
    if (xQueueReceiveFromISR(s_port[spi].spi_buf.tx_queue.handle, (uint8_t *)&tx_data,
                             &xTaskWoken)) {
      SPI_I2S_SendData(s_port[spi].base, tx_data);
    } else {
      // We have sent all data, so close spi
      // TODO(mitchellostler): Add Busy check
      SPI_I2S_ITConfig(s_port[spi].base, SPI_I2S_IT_ERR | SPI_I2S_IT_TXE, DISABLE);
      SPI_Cmd(s_port[spi].base, DISABLE);
      xSemaphoreGiveFromISR(s_port[spi].spi_buf.mutex.handle, &xTaskWoken);
    }
  }
  if (SPI_I2S_GetITStatus(s_port[spi].base, SPI_I2S_IT_RXNE) == SET) {
    uint16_t rx_data = SPI_I2S_ReceiveData(s_port[spi].base);
    xQueueSendFromISR(s_port[spi].spi_buf.rx_queue.handle, (uint8_t *)&rx_data, &xTaskWoken);
  }
  if (SPI_I2S_GetITStatus(s_port[spi].base, SPI_IT_MODF) == SET) {
    // Mode fault - If this is happening we most likely have an invalid hardware setup
    SPI_Cmd(s_port[spi].base, ENABLE);
  } else {
    // Currently do not handle other  errors in SPI, since for our purposes
    // (single master, master only) they should not occur.
    // CRC may be a good addition, but all transactions are short currently
    return;
  }

  portYIELD_FROM_ISR(xTaskWoken);
}

void SPI1_IRQHandler(void) {
  prv_spi_irq_handler(SPI_PORT_1);
}

void SPI2_IRQHandler(void) {
  prv_spi_irq_handler(SPI_PORT_2);
}
