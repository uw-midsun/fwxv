#pragma once

#include <stdint.h>

#include "status.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

typedef void (*TimerCallback)(void);

void hw_timer_init(void);

void hw_timer_delay_us(uint32_t us);

void hw_timer_callback_us(uint32_t us, TimerCallback callback_function);
