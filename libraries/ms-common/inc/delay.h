#pragma once
// Delay function library
//
// Can only be called in a task, calling in main/initialize
// functions will cause undefined behaviour
//
// Uses vTaskDelay from FreeRTOS to allow other tasks to run
// during the delay.

#include <stdint.h>

/**
 * @brief delay the current task by `t` milliseconds,
 * allows lower priority tasks to run while the current task is delayed.
 *
 * @param t milliseconds to delay
 */
void delay_ms(uint32_t t);

/**
 * @brief non-blocking delay the current task by `t` milliseconds,
 * while task is delayed with non-blocking delay, lower priorities tasks will not be able to run.
 *
 * @param t milliseconds to delay
 */
void non_blocking_delay_ms(uint32_t t);

/**
 * @brief delay the current task by `t` seconds.
 *
 * @param time uint32_t seconds to delay (time * 1000 should not exceed UINT32_MAX).
 */
#define delay_s(time) delay_ms((time)*1000)
