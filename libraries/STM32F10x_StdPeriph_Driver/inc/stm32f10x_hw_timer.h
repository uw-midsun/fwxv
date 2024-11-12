#pragma once
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "status.h"
#include <stdint.h>


typedef void (* HardwareTimerCallback) (TIM_TypeDef *TIMX);


StatusCode hardware_timer_init_and_start(uint32_t duration_us);

StatusCode hardware_timer_interrupt(uint32_t duration_us);