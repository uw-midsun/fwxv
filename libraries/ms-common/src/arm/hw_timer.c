#include "hw_timer.h"

#include "log.h"
#include "stm32f10x.h"

#define hw_timer_delay_ms(ms) hw_timer_delay_us((ms)*1000)
#define hw_timer_callback_ms(ms, callback_function) \
  hw_timer_delay_us((ms)*1000, (callback_function))

void hw_timer_init(void) {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_Cmd(TIM2, DISABLE);
  TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef timer_init = {
    .TIM_Prescaler = (clocks.PCLK1_Frequency / 1000000) - 1,  // 1 Mhz
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = UINT16_MAX,
    .TIM_ClockDivision = TIM_CKD_DIV1,
  };
  TIM_TimeBaseInit(TIM2, &timer_init);
  TIM_Cmd(TIM2, ENABLE);
}

void hw_timer_delay_us(uint32_t us) {
    TIM_SetCounter(TIM2, 0);

    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Enable);

    TIM_SetCompare1(TIM2, us);

    TIM_Cmd(TIM2, ENABLE);

    while (!TIM_GetFlagStatus(TIM2, TIM_FLAG_CC1)) {
        LOG_DEBUG("DELAYING\n");
    }

    // Clear the capture/compare interrupt flag
    TIM_ClearFlag(TIM2, TIM_FLAG_CC1);

    // Disable capture/compare channel 1 of Timer 2
    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Disable);

    // Disable Timer 2
    TIM_Cmd(TIM2, DISABLE);
}

void hw_timer_callback_us(uint32_t us, TimerCallback callback_function) {
  hw_timer_delay_us(us);

  if (callback_function != NULL) callback_function();
}
