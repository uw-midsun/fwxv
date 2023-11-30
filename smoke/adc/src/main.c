#include <stdio.h>
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "interrupt.h"
#include "log.h"
#include "tasks.h"

#define ADC_TIMEOUT_MS 100

static const GpioAddress adc_addy[] = {
  { .port = GPIO_PORT_A, .pin = 0 },   
  { .port = GPIO_PORT_A, .pin = 1 },   
  { .port = GPIO_PORT_A, .pin = 2 },
  { .port = GPIO_PORT_A, .pin = 3 },
  { .port = GPIO_PORT_A, .pin = 4 },
  { .port = GPIO_PORT_A, .pin = 5 },
  { .port = GPIO_PORT_A, .pin = 6 },
  { .port = GPIO_PORT_A, .pin = 7 },
  { .port = GPIO_PORT_B, .pin = 0 },
  { .port = GPIO_PORT_B, .pin = 1 },
};

TASK(smoke_adc_task, TASK_STACK_512) {
   for (uint8_t i = 0; i < SIZEOF_ARRAY(adc_addy); i++) {
      gpio_init_pin(&adc_addy[i], GPIO_ANALOG, GPIO_STATE_LOW);
      adc_add_channel(adc_addy[i]);
   }

   adc_init();

   while(true) {
      uint16_t data = 0;
      adc_run();
      for (uint8_t i = 0; i < SIZEOF_ARRAY(adc_addy); i++) {
        adc_read_converted(adc_addy[i], &data);
        LOG_DEBUG("%d%d: %d\n\r", adc_addy[i].port, adc_addy[i].pin, data);
      }
      delay_ms(1000);
   }
}

int main() {
   tasks_init();
   interrupt_init();
   gpio_init();
   log_init();

   tasks_init_task(smoke_adc_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   return 0;
}

