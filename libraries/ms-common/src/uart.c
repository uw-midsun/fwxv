#include "uart.h"
#include <string.h>
#include "mutex.h"
#include "stm32f0xx.h"
#include "stm32f0xx_interrupt.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_usart.h"

static Queue tx_queue_port_1;
static Queue tx_queue_port_2;
static Queue tx_queue_port_3;
static Queue tx_queue_port_4;

static uint8_t tx_buf_port_1[UART_MAX_BUFFER_LEN];
static uint8_t tx_buf_port_2[UART_MAX_BUFFER_LEN];
static uint8_t tx_buf_port_3[UART_MAX_BUFFER_LEN];
static uint8_t tx_buf_port_4[UART_MAX_BUFFER_LEN];

static Queue rx_queue_port_1;
static Queue rx_queue_port_2;
static Queue rx_queue_port_3;
static Queue rx_queue_port_4;

static uint8_t rx_buf_port_1[UART_MAX_BUFFER_LEN];
static uint8_t rx_buf_port_2[UART_MAX_BUFFER_LEN];
static uint8_t rx_buf_port_3[UART_MAX_BUFFER_LEN];
static uint8_t rx_buf_port_4[UART_MAX_BUFFER_LEN];

static Mutex tx_data_port_1;
static Mutex tx_data_port_2;
static Mutex tx_data_port_3;
static Mutex tx_data_port_4;

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  uint32_t irq;
  Queue *tx_queue;
  Queue *rx_queue;
  uint8_t *tx_buf;
  uint8_t *rx_buf;
  USART_TypeDef *base;
  Mutex *tx_mutex;
} UartPortData;

static UartPortData s_port[] = {
  [UART_PORT_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                    .periph = RCC_APB2Periph_USART1,
                    .irq = USART1_IRQn,
                    .tx_mutex = &tx_data_port_1,
                    .tx_queue = &tx_queue_port_1,
                    .rx_queue = &rx_queue_port_1,
                    .tx_buf = tx_buf_port_1,
                    .rx_buf = rx_buf_port_1,
                    .base = USART1 },
  [UART_PORT_2] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART2,
                    .irq = USART2_IRQn,
                    .tx_mutex = &tx_data_port_2,
                    .tx_queue = &tx_queue_port_2,
                    .rx_queue = &rx_queue_port_2,
                    .tx_buf = tx_buf_port_2,
                    .rx_buf = rx_buf_port_2,
                    .base = USART2 },
  [UART_PORT_3] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART3,
                    .tx_mutex = &tx_data_port_3,
                    .tx_queue = &tx_queue_port_3,
                    .rx_queue = &rx_queue_port_3,
                    .tx_buf = tx_buf_port_2,
                    .rx_buf = rx_buf_port_2,
                    .irq = USART3_4_IRQn,
                    .base = USART3 },
  [UART_PORT_4] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART4,
                    .tx_mutex = &tx_data_port_4,
                    .tx_queue = &tx_queue_port_4,
                    .rx_queue = &rx_queue_port_4,
                    .tx_buf = tx_buf_port_2,
                    .rx_buf = rx_buf_port_2,
                    .irq = USART3_4_IRQn,
                    .base = USART4 },
};

static void prv_handle_irq(UartPort uart);

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  // Reserve USART Port 1 for Retarget.c
  if (uart == UART_PORT_1) return STATUS_CODE_INVALID_ARGS;

  mutex_init(s_port[uart].tx_mutex);

  s_port[uart].rcc_cmd(s_port[uart].periph, ENABLE);

  s_port[uart].tx_queue->item_size = sizeof(uint8_t);
  s_port[uart].tx_queue->num_items = UART_MAX_BUFFER_LEN;
  s_port[uart].tx_queue->storage_buf = s_port[uart].tx_buf;
  queue_init(s_port[uart].tx_queue);

  s_port[uart].rx_queue->item_size = sizeof(uint8_t);
  s_port[uart].rx_queue->num_items = UART_MAX_BUFFER_LEN;
  s_port[uart].rx_queue->storage_buf = s_port[uart].rx_buf;
  queue_init(s_port[uart].rx_queue);

  GpioSettings gpio_settings = {
    .alt_function = settings->alt_fn,
    .resistor = GPIO_RES_PULLUP,
  };

  gpio_init_pin(&settings->tx, &gpio_settings);
  gpio_init_pin(&settings->rx, &gpio_settings);

  USART_InitTypeDef usart_init;
  USART_StructInit(&usart_init);
  usart_init.USART_BaudRate = BAUDRATE;
  USART_Init(s_port[uart].base, &usart_init);

  USART_ClearITPendingBit(s_port[uart].base, USART_FLAG_TXE);
  USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
  USART_ITConfig(s_port[uart].base, USART_IT_RXNE, ENABLE);

  stm32f0xx_interrupt_nvic_enable(s_port[uart].irq, INTERRUPT_PRIORITY_NORMAL);

  USART_Cmd(s_port[uart].base, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t *len) {
  // Aquire tx mutex
  if (mutex_lock(s_port[uart].tx_mutex, 0) == STATUS_CODE_TIMEOUT)
    return STATUS_CODE_RESOURCE_EXHAUSTED;

  StatusCode status;
  for (uint8_t i = 0; i < *len; i++) {
    status = queue_send(s_port[uart].tx_queue, &data[i], 0);
    if (status != STATUS_CODE_OK) {
      *len = i;
      break;
    }
  }

  if (USART_GetFlagStatus(s_port[uart].base, USART_FLAG_TXE) == SET)
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, ENABLE);
  return status;
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t *len) {
  StatusCode status;
  for (uint8_t i = 0; i < *len; i++) {
    status = queue_receive(s_port[uart].rx_queue, &data[i], 0);
    if (status != STATUS_CODE_OK) {
      *len = i;
      break;
    }
  }

  // If the buffer was filled and items remain in the queue
  uint8_t buf;
  if (queue_peek(s_port[uart].rx_queue, &buf, 0) == STATUS_CODE_OK) status = STATUS_CODE_INCOMPLETE;

  return status;
}

static void prv_handle_irq(UartPort uart) {
  if (USART_GetITStatus(s_port[uart].base, USART_IT_TXE) == SET) {
    uint8_t tx_data = 0;
    // If tx queue is empty, disable   tx interrupts and return
    if (xQueueReceiveFromISR(s_port[uart].tx_queue->handle, &tx_data, pdFALSE) == pdFALSE) {
      // Release tx mutex
      mutex_unlock(s_port[uart].tx_mutex);
      USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
    } else {
      USART_SendData(s_port[uart].base, tx_data);
    }
    USART_ClearITPendingBit(s_port[uart].base, USART_IT_TXE);
  }

  if (USART_GetITStatus(s_port[uart].base, USART_IT_RXNE) == SET) {
    uint8_t rx_data = USART_ReceiveData(s_port[uart].base);
    if (xQueueSendFromISR(s_port[uart].rx_queue->handle, &rx_data, pdFALSE) == errQUEUE_FULL) {
      // Drop oldest data
      uint8_t buf = 0;
      xQueueReceiveFromISR(s_port[uart].tx_queue->handle, &buf, pdFALSE);
      xQueueSendFromISR(s_port[uart].rx_queue->handle, &rx_data, pdFALSE);
    }
  }

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
