#include <stdbool.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "uart.h"
// #include "tasks.h"

// static const GpioAddress leds[] = {
//   { .port = GPIO_PORT_B, .pin = 5 },   //
//   { .port = GPIO_PORT_B, .pin = 4 },   //
//   { .port = GPIO_PORT_B, .pin = 3 },   //
//   { .port = GPIO_PORT_A, .pin = 15 },  //
// };
// 
// void UART1_Send(uint8_t *data, uint16_t length)
// {
//     for(int i = 0; i < length; i++)
//     {
//         // Wait until the transmit buffer is empty
//         while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
// 
//         // Send the data
//         USART_SendData(USART1, data[i]);
//     }
// }
// 
// TASK(leds_task, TASK_STACK_512) {
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
//     gpio_init_pin(&leds[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
//   }
//   UartSettings set = {
//     .tx = { .port = GPIO_PORT_B, .pin = 6 },
//     .rx = { .port = GPIO_PORT_B, .pin = 7 },
//     .baudrate = 12500,
//   };
//   uart_init(UART_PORT_1, &set); 
//   unsigned char to_send[] = "helloworld\n";
// 
// 
//   while (true) {
// #ifdef MS_PLATFORM_X86
//     LOG_DEBUG("blink\n");
// #endif
//     size_t len = sizeof(to_send);
//     UART1_Send((uint8_t*)to_send, sizeof(to_send));
//     // uart_tx(UART_PORT_1, to_send, &len);
//     for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
//       gpio_toggle_state(&leds[i]);
//       delay_ms(50);
//     }
//   }
// }

void UART1_Init(uint32_t baudrate)
{
    // Enable clock for UART1 and GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // Configure UART1 pins
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio);

    // Configure UART1
    USART_InitTypeDef uart;
    uart.USART_BaudRate = baudrate;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &uart);

    // Enable UART1
    USART_Cmd(USART1, ENABLE);
}

void UART1_Send(uint8_t *data, uint16_t length)
{
    for(int i = 0; i < length; i++)
    {
        // Wait until the transmit buffer is empty
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

        // Send the data
        USART_SendData(USART1, data[i]);
    }
}

int main(void) {
  uint8_t data[1000];
  UART1_Init(112500);
  UART1_Send(data, 1000);


   
  return 0;
}
