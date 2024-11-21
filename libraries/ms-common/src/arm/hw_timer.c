
#include "hw_timer.h"


typedef struct {
    TIM_TypeDef *TIMX;
    bool state;
    uint32_t ABP_periph; 
    HardwareTimerCallback callback;
} HWTimerData;

//Timers configuration 
static HWTimerData s_timer[3] = {
    {TIM2, false, RCC_APB1Periph_TIM2, NULL},
    {TIM3, false, RCC_APB1Periph_TIM3, NULL},
    {TIM4, false, RCC_APB1Periph_TIM4, NULL}
};

// HardwareTimerCallback htim2, htim3, htim4;
void TIM2_IRQHandler(void) {
    if (s_timer[0].callback){
        s_timer[0].callback();
    }
}
void TIM3_IRQHandler(void) {
    if (s_timer[1].callback){
        s_timer[1].callback();
    }
}
void TIM4_IRQHandler(void) {
    if (s_timer[2].callback){
        s_timer[2].callback();
    }
}


StatusCode hardware_timer_init_and_start(uint32_t duration_us, HardwareTimerCallback callback){
    //Timer array for which timers are in use.

    // TIM_TypeDef *TIMX;
    for (int i = 0; i < 3; i++){
        if (!s_timer[i].state){
            s_timer[i].state = true;
            s_timer[i].callback = callback;
            
            
            RCC_APB1PeriphClockCmd(s_timer[i].ABP_periph, ENABLE);
            
            
            TIM_TimeBaseInitTypeDef config = {
            72 - 1, // replace this using clock speed / 1000000
            TIM_CounterMode_Up, 
            duration_us - 1, 
            TIM_CKD_DIV1, 
            0
            };

            TIM_TimeBaseInit(s_timer[i].TIMX, &config); // configs timer


            // enable update interrupt for timer
            TIM_ITConfig(s_timer[i].TIMX, TIM_IT_Update, ENABLE);


            // enable timer
            TIM_Cmd(s_timer[i].TIMX, ENABLE);

            return STATUS_CODE_OK;

        } else {
            //All timers are in use
            return STATUS_CODE_RESOURCE_EXHAUSTED;
        }
    }
}

