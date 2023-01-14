// #include <stdio.h>
// 
// #include "gpio.h"
// #include "stm32f10x.h"
// #include "stm32f10x_rcc.h"

int main() {
  
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  // 
  // GPIO_InitTypeDef init_struct = {
  //   .GPIO_Pin = 0x1 << 14,
  //   .GPIO_Speed = GPIO_Speed_50MHz,  // Default to fastes speed
  //   .GPIO_Mode = GPIO_Mode_Out_PP,
  // };

  // BitAction state = Bit_RESET;
  // GPIO_Init(GPIOA, &init_struct);
  // GPIO_WriteBit(GPIOA, 0x01 << 14, (BitAction)state);

  // init_struct.GPIO_Pin = 0x01 << 3;
  // GPIO_Init(GPIOB, &init_struct);
  // GPIO_WriteBit(GPIOB, 0x01 << 3, (BitAction)state);

  // init_struct.GPIO_Pin = 0x01 << 4;
  // GPIO_Init(GPIOB, &init_struct);
  // GPIO_WriteBit(GPIOB, 0x01 << 3, (BitAction)state);

  // init_struct.GPIO_Pin = 0x01 << 5;
  // GPIO_Init(GPIOB, &init_struct);
  int a;
  while(1){
  a++;
  a--;
  }
  
}

