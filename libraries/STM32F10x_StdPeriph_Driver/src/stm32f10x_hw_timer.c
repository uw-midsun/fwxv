
#include "stm32f10x_hw_timer.h"



StatusCode hardware_timer_init_and_start(uint32_t duration_us, HardwareTimerCallback func, TIM_TypeDef *TIMX){
    //RCC
    RCC_ClocksTypeDef *clock;
    RCC_GetClocksFreq(clock);
    uint16_t ClocksFreq = clock->SYSCLK_Frequency;

    //Base init of TimeBaseInitTypeDef
}
