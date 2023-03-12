#include "uart.h"

#include <string.h>

#include "semaphore.h"
#include "stm32f10x.h"
#include "stm32f10x_interrupt.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

typedef struct UartPortQueue {
  uint8_t tx_buf[UART_MAX_BUFFER_LEN];
  Queue tx_queue;
  uint8_t rx_buf[UART_MAX_BUFFER_LEN];
  Queue rx_queue;
} UartPortQueue;

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  uint32_t irq;
  USART_TypeDef *base;
  bool initialized;
} UartPortData;

static UartPortQueue s_port_queues[NUM_UART_PORTS];

static UartPortData s_port[] = {
  [UART_PORT_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                    .periph = RCC_APB2Periph_USART1,
                    .irq = USART1_IRQn,
                    .base = USART1 },
  [UART_PORT_2] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART2,
                    .irq = USART2_IRQn,
                    .base = USART2 },
  [UART_PORT_3] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART3,
                    .irq = USART3_IRQn,
                    .base = USART3 },
};

static void prv_handle_irq(UartPort uart);

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  // Reserve USART Port 1 for Retarget.c
  // if (uart == UART_PORT_1 || settings == NULL) return STATUS_CODE_INVALID_ARGS;
  if (s_port[uart].initialized) return STATUS_CODE_RESOURCE_EXHAUSTED;

  s_port[uart].rcc_cmd(s_port[uart].periph, ENABLE);

  s_port_queues[uart].tx_queue.item_size = sizeof(uint8_t);
  s_port_queues[uart].tx_queue.num_items = UART_MAX_BUFFER_LEN;
  s_port_queues[uart].tx_queue.storage_buf = s_port_queues[uart].tx_buf;
  queue_init(&s_port_queues[uart].tx_queue);

  s_port_queues[uart].rx_queue.item_size = sizeof(uint8_t);
  s_port_queues[uart].rx_queue.num_items = UART_MAX_BUFFER_LEN;
  s_port_queues[uart].rx_queue.storage_buf = s_port_queues[uart].rx_buf;
  queue_init(&s_port_queues[uart].rx_queue);

  gpio_init_pin(&settings->tx, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->rx, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

  USART_InitTypeDef usart_init;
  USART_StructInit(&usart_init);
  usart_init.USART_BaudRate = settings->baudrate;
  USART_Init(s_port[uart].base, &usart_init);

  // ITPendingBit is cleared once a pending interrupt has been serviced
  // Init with no pending interrupts
  USART_ClearITPendingBit(s_port[uart].base, USART_FLAG_TXE);
  // Init with disabled TX interrupts otherwise we will get TX
  // buffer empty continuously when there is no data to send
  USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
  // Init with enabled RX interrupts becuase they are called only
  // when there is data to be received
  USART_ITConfig(s_port[uart].base, USART_IT_RXNE, ENABLE);

  stm32f10x_interrupt_nvic_enable(s_port[uart].irq, INTERRUPT_PRIORITY_NORMAL);

  USART_Cmd(s_port[uart].base, ENABLE);
  s_port[uart].initialized = true;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t *len) {
  if (data == NULL || len == NULL) return STATUS_CODE_INVALID_ARGS;
  StatusCode status = STATUS_CODE_OK;
  // Send all data to queue;
  for (uint8_t i = 0; i < *len; i++) {
    if (queue_send(&s_port_queues[uart].tx_queue, &data[i], 0) != STATUS_CODE_OK) {
      *len = i;
      status = STATUS_CODE_INCOMPLETE;
      break;
    }
  }

  if (USART_GetFlagStatus(s_port[uart].base, USART_FLAG_TXE) == SET) {
    uint8_t tx_data = 0;
    StatusCode cmpl = queue_receive(&s_port_queues[uart].tx_queue, &tx_data, 0);

    if (cmpl == STATUS_CODE_EMPTY) {
      USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
      return status;
    }
    USART_SendData(s_port[uart].base, tx_data);
    // Enable TX Interrupts to begin transmission
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, ENABLE);
  }
  return status;
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t *len) {
  if (data == NULL || len == NULL) return STATUS_CODE_INVALID_ARGS;
  StatusCode status;
  for (uint8_t i = 0; i < *len; i++) {
    status = queue_receive(&s_port_queues[uart].rx_queue, &data[i], 0);
    if (status != STATUS_CODE_OK) {
      *len = i;
      return STATUS_CODE_INCOMPLETE;
    }
  }

  return STATUS_CODE_OK;
}

static void prv_handle_irq(UartPort uart) {
  // Check that the transmit data register is empty
  if (USART_GetITStatus(s_port[uart].base, USART_IT_TXE) == SET) {
    uint8_t tx_data = 0;
    // If tx queue is empty, disable tx interrupts and return
    if (xQueueReceiveFromISR(s_port_queues[uart].tx_queue.handle, &tx_data, pdFALSE) == pdFALSE) {
      // Disable TX Interrupts to end transmission
      USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
    } else {
      USART_SendData(s_port[uart].base, tx_data);
    }
    USART_ClearITPendingBit(s_port[uart].base, USART_IT_TXE);
  }

  // Check that the receive data register is not empty
  if (USART_GetITStatus(s_port[uart].base, USART_IT_RXNE) == SET) {
    uint8_t rx_data = USART_ReceiveData(s_port[uart].base);
    if (xQueueSendFromISR(s_port_queues[uart].rx_queue.handle, &rx_data, pdFALSE) ==
        errQUEUE_FULL) {
      // Drop oldest data if queue is full
      uint8_t buf = 0;
      xQueueReceiveFromISR(s_port_queues[uart].tx_queue.handle, &buf, pdFALSE);
      xQueueSendFromISR(s_port_queues[uart].rx_queue.handle, &rx_data, pdFALSE);
    }
  }

  // Overrun error occurs when another byte of data arrives before the
  // previous byte has been read from the UART's receive buffer
  // Clear overrun flag
  USART_ClearITPendingBit(s_port[uart].base, USART_IT_ORE);
}

void USART1_IRQHandler(void) {
  prv_handle_irq(UART_PORT_1);
}

void USART2_IRQHandler(void) {
  prv_handle_irq(UART_PORT_2);
}

void USART3_4_IRQHandler(void) {
  prv_handle_irq(UART_PORT_3);
  prv_handle_irq(UART_PORT_4);
}
