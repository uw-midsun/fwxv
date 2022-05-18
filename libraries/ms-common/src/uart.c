#include "uart.h"
#include <string.h>
#include "mutex.h"
#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_usart.h"

// basic idea: tx is stored in a queue, interrupt-driven
// rx data is received to a queue, the task which initialized it
// has access to the queue and can read at its leisure

static Queue tx_queue;

static Mutex tx_data_port_1;
static Mutex tx_data_port_2;
static Mutex tx_data_port_3;
static Mutex tx_data_port_4;

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  uint32_t irq;
  Queue rx_queue;
  USART_TypeDef *base;
  UartStorage *storage;
  Mutex *tx_mutex;
} UartPortData;

static UartPortData s_port[] = {
  [UART_PORT_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                    .periph = RCC_APB2Periph_USART1,
                    .irq = USART1_IRQn,
                    .tx_mutex = &tx_data_port_1,
                    .base = USART1 },
  [UART_PORT_2] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART2,
                    .irq = USART2_IRQn,
                    .tx_mutex = &tx_data_port_2,
                    .base = USART2 },
  [UART_PORT_3] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART3,
                    .tx_mutex = &tx_data_port_3,
                    .irq = USART3_4_IRQn,
                    .base = USART3 },
  [UART_PORT_4] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART4,
                    .tx_mutex = &tx_data_port_4,
                    .irq = USART3_4_IRQn,
                    .base = USART4 },
};

static void prv_tx_pop(UartPort uart);
static void prv_rx_push(UartPort uart);

static void prv_handle_irq(UartPort uart);

StatusCode uart_init(UartPort uart, UartSettings *settings, UartStorage *storage) {
  // Reserve USART Port 1 for Retarget.c
  if (uart == UART_PORT_1) return STATUS_CODE_INVALID_ARGS;

  mutex_init(s_port[uart].tx_mutex);

  s_port[uart].rcc_cmd(s_port[uart].periph, ENABLE);

  s_port[uart].storage = storage;

  tx_queue.item_size = sizeof(uint8_t);
  tx_queue.num_items = UART_MAX_BUFFER_LEN;
  tx_queue.storage_buf = s_port[uart].storage->tx_buf;

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

StatusCode uart_tx(UartPort uart, uint8_t *tx_data, uint8_t len, uint16_t ms_to_wait) {
  // Aquire tx mutex
  if (mutex_lock(s_port[uart].tx_mutex, ms_to_wait) == STATUS_CODE_TIMEOUT)
    return STATUS_CODE_RESOURCE_EXHAUSTED;

  for (int i = 0; i < len; i++) {
    queue_send(&tx_queue, &tx_data[i], 0);
  }

  if (USART_GetFlagStatus(s_port[uart].base, USART_FLAG_TXE) == SET) {
    prv_tx_pop(uart);
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, ENABLE);
  }

  return STATUS_CODE_OK;
}

StatusCode uart_rx_init(UartRxSettings *settings) {
  USART_ITConfig(s_port[settings->uart].base, USART_IT_RXNE, ENABLE);
  s_port[settings->uart].rx_queue = *settings->rx_queue;

  return STATUS_CODE_OK;
}

static void prv_tx_pop(UartPort uart) {
  uint8_t tx_data = 0;
  // If tx queue is empty, disable tx interrupts and return
  if (queue_receive(&tx_queue, &tx_data, 0) == STATUS_CODE_EMPTY) {
    // Release tx mutex
    mutex_unlock(s_port[uart].tx_mutex);
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
    return;
  }
  USART_SendData(s_port[uart].base, tx_data);
}

static void prv_rx_push(UartPort uart) {
  UartStorage *storage = s_port[uart].storage;

  uint8_t rx_data = USART_ReceiveData(s_port[uart].base);
  // If the queue is full drop oldest data
  if (queue_send(&s_port[uart].rx_queue, &rx_data, 0) == STATUS_CODE_RESOURCE_EXHAUSTED) {
    uint8_t outstr;
    queue_receive(&s_port[uart].rx_queue, &outstr, 0);
    queue_send(&s_port[uart].rx_queue, &rx_data, 0);
  }
}

static void prv_handle_irq(UartPort uart) {
  if (USART_GetITStatus(s_port[uart].base, USART_IT_TXE) == SET) {
    prv_tx_pop(uart);
    USART_ClearITPendingBit(s_port[uart].base, USART_IT_TXE);
  }

  if (USART_GetITStatus(s_port[uart].base, USART_IT_RXNE) == SET) {
    prv_rx_push(uart);
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
