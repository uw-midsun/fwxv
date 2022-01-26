#include "FreeRTOS.h"
#include "task.h"

#include "delay.h"

void delay_ms(uint32_t t) {
  const TickType_t xDelay = t / portTICK_PERIOD_MS;
  vTaskDelay(xDelay);
}
