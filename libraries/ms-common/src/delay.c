#include "delay.h"

#include "FreeRTOS.h"
#include "task.h"

void delay_ms(uint32_t t) {
  vTaskDelay(pdMS_TO_TICKS(t));
}
