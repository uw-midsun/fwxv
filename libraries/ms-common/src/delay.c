#include "delay.h"

#include "FreeRTOS.h"
#include "log.h"
#include "task.h"

void delay_ms(uint32_t t) {
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    LOG_CRITICAL("Error: Scheduler is not running!");
  } else {
    vTaskDelay(pdMS_TO_TICKS(t));
  }
}

void non_blocking_delay_ms(uint32_t t) {
  TickType_t ticks = pdMS_TO_TICKS(t) + xTaskGetTickCount();
  while (xTaskGetTickCount() < ticks) {
  }
}
