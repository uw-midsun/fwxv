#pragma once
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "status.h"
#include <stdint.h>
#include <stdbool.h>


typedef void (*HardwareTimerCallback) (void);


StatusCode hardware_timer_init_and_start(uint16_t duration_us, HardwareTimerCallback callback);

StatusCode hardware_timer_interrupt(uint16_t duration_us);

void hardware_callback(void);