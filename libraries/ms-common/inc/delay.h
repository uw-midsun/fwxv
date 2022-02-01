#pragma once
// Delay function library
//
// Can only be called in a task, calling in main/initialize
// functions will cause undefined behaviour
//
// Uses vTaskDelay from FreeRTOS to allow other tasks to run
// during the delay.

#include <stdint.h>

// Delay for a period in milliseconds.
void delay_ms(uint32_t t);

// Delay for a period in seconds.
#define delay_s(time) delay_ms((time)*1000)
