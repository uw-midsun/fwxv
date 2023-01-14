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

int main(void) {
  for(int i = 0; i < 1000; i++) {
    printf("Hello World\n");
  }
    
  return 0;
}
