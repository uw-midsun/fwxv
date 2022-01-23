#include "delay.h"

void delay_us(uint32_t t) {
  const TickType_t xDelay = (t / 1000) / portTICK_PERIOD_MS;
  vTaskDelay(xDelay);
}