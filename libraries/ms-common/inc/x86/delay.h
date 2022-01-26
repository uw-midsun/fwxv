#pragma once
// Delay function library
//
// Uses vTaskDelay from FreeRTOS to allow other tasks to run
// during the delay.

// Delay for a period in milliseconds.
void delay_ms(uint32_t t);

// Delay for a period in seconds.
#define delay_s(time) delay_ms((time)*1000)
