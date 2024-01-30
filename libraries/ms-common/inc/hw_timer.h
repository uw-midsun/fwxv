#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "status.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_interrupt.h"

#define hw_timer_delay_ms(ms) hw_timer_delay_us((ms) * 1000)

void hw_timer_init(void);

void hw_timer_delay_us(uint32_t us);

