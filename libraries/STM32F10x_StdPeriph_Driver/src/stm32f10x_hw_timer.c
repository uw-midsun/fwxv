
#include "stm32f10x_hw_timer.h"



StatusCode hardware_timer_init_and_start(uint32_t duration_us){
    //Timer array for which timers are in use.
    static bool timers[3] = {false, false, false};

    TIM_TypeDef *TIMX;
    uint_32t ABP_periph;


    if (!timers[0]){
        TIMX = TIM2;
        timers[0] = true;
        ABP_periph = RCC_APB1Periph_TIM2;
        
    } else if (!timers[1]){
        TIMX = TIM3;
        timers[1] = true;
        ABP_periph = RCC_APB1Periph_TIM2;
    } else if (!timers[2]){
        TIMX = TIM4; 
        timers[2] = true;
        ABP_periph = RCC_APB1Periph_TIM2;
    } else {
        //All timers are in use
        return STATUS_CODE_RESOURCE_EXHAUSTED;
    }

    
    
    


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
