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
  uint32_t start_count = TIM_GetCounter(TIM2);
  uint32_t elapsed_time = 0;

  while (elapsed_time < us) {
    uint32_t current_count = TIM_GetCounter(TIM2);
    if (current_count <= start_count) {
      elapsed_time += (UINT16_MAX + 1 - start_count) + current_count;
    } else {
      elapsed_time += current_count - start_count;
    }
    start_count = current_count;
  }
}

void hw_timer_callback_us(uint32_t us, TimerCallback callback_function) {
  hw_timer_delay_us(us);

  if (callback_function != NULL) callback_function();
}
