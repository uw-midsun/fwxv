#include "delay.h"

#include "FreeRTOS.h"
#include "task.h"

void delay_ms(uint32_t t) {
  vTaskDelay(pdMS_TO_TICKS(t));
}

void non_blocking_delay_ms(uint32_t t) {
  TickType_t ticks = pdMS_TO_TICKS(t) + xTaskGetTickCount();
  while (xTaskGetTickCount() < ticks) {
  }
}