
#include "hw_timer.h"
#include "stm32f10x_interrupt.h"
#include <stddef.h>


typedef struct {
    TIM_TypeDef *TIMX;
    bool state;
    uint32_t ABP_periph; 
    HardwareTimerCallback callback;
    uint8_t irq_channel;
    InterruptPriority priority; // Interrupt priority for the timer


} HWTimerData;

void placeholder(){}

//Timers configuration 
static HWTimerData s_timer[3] = {
    {TIM2, false, RCC_APB1Periph_TIM2, NULL, TIM2_IRQn, INTERRUPT_PRIORITY_HIGH},
    {TIM3, false, RCC_APB1Periph_TIM3, NULL, TIM3_IRQn, INTERRUPT_PRIORITY_HIGH},
    {TIM4, false, RCC_APB1Periph_TIM4, NULL, TIM4_IRQn, INTERRUPT_PRIORITY_HIGH}
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


StatusCode hardware_timer_init_and_start(uint16_t duration_us, HardwareTimerCallback callback){

    // TIM_TypeDef *TIMX;
    for (int i = 0; i < 3; i++){
        if (!s_timer[i].state){
            s_timer[i].state = true;
            s_timer[i].callback = callback;
            
            
            RCC_APB1PeriphClockCmd(s_timer[i].ABP_periph, ENABLE);
            
            RCC_ClocksTypeDef clocks;
            RCC_GetClocksFreq(&clocks);
            uint32_t prescaler = (clocks.PCLK2_Frequency)/1000000;

            
            TIM_TimeBaseInitTypeDef config = {
            (uint16_t) prescaler,
            TIM_CounterMode_Up, 
            duration_us - 1, 
            TIM_CKD_DIV1, 
            0
            };

            TIM_TimeBaseInit(s_timer[i].TIMX, &config); // configs timer


            // enable update interrupt for timer
            TIM_ITConfig(s_timer[i].TIMX, TIM_IT_Update, ENABLE);


            // Enable NVIC interrupt for the timer
            stm32f10x_interrupt_nvic_enable(s_timer[i].irq_channel, s_timer[i].priority);

            // enable timer
            TIM_Cmd(s_timer[i].TIMX, ENABLE);


            return STATUS_CODE_OK;

        } 
    }
    return STATUS_CODE_RESOURCE_EXHAUSTED;

}

