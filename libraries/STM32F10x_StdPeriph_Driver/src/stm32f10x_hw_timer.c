
#include "stm32f10x_hw_timer.h"



StatusCode hardware_timer_init(uint32_t duration_us, TIM_TypeDef *TIMX){
    //RCC
    RCC_ClocksTypeDef *clock;
    RCC_GetClocksFreq(clock);
    


    //Base init of TimeBaseInitTypeDef
    TIM_TimeBaseInitTypeDef config = {
            clock->SYSCLK_Frequency/1000000, // replace this using clock speed / 1000000
            TIM_CounterMode_Up, 
            duration_us - 1, 
            TIM_CKD_DIV1, 
            0
            };

    TIM_TimeBaseInit(TIMX, &config); // configs timer

}

StatusCode hardware_timer_start(HardwareTimerCallback *func, TIM_TypeDef *TIMX){
    
}
