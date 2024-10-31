
#include "stm32f10x_hw_timer.h"



StatusCode hardware_timer_init_and_start(uint32_t duration_us, HardwareTimerCallback func, TIM_TypeDef *TIMX){
    //RCC

        struct TIM_TimeBaseInitTypeDef config = {
                prescaler, 
                TIM_CounterMode_Up, 
                duration_us - 1, 
                TIM_CLOCKDIVISION_DIV1, 
                0};

        

}