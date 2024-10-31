#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "status.h"
#include <stdint.h>


typedef void (* HardwareTimerCallback) (TIM_TypeDef *TIMx);













StatusCode hardware_timer_init(uint32_t duration_us, TIM_TypeDef *TIMX);

StatusCode hardware_timer_start(HardwareTimerCallback *func, TIM_TypeDef *TIMX);