
#include "stm32f10x_hw_timer.h"



StatusCode hardware_timer_init_and_start(uint32_t duration_us, TIM_TypeDef *TIMX){
    //Configure ABP1 / ABP2 functionality

    TIM_Cmd(TIMX, ENABLE);


    //Base init of TimeBaseInitTypeDef
    TIM_TimeBaseInitTypeDef config = {
            72 - 1, // replace this using clock speed / 1000000
            TIM_CounterMode_Up, 
            duration_us - 1, 
            TIM_CKD_DIV1, 
            0
        };
    

    TIM_TimeBaseInit(TIMX, &config); // configs timer

}
