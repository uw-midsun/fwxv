#include <stdio.h>
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "log.h"
#include "tasks.h"


static const GpioAddress adc_addy[] = {
  { .port = GPIO_PORT_A, .pin = 0 },   
  { .port = GPIO_PORT_A, .pin = 1 },   
  { .port = GPIO_PORT_A, .pin = 2 },
  { .port = GPIO_PORT_A, .pin = 4 },
  { .port = GPIO_PORT_A, .pin = 5 },
  { .port = GPIO_PORT_A, .pin = 6 },
  { .port = GPIO_PORT_A, .pin = 7 },
  { .port = GPIO_PORT_B, .pin = 0 },
  { .port = GPIO_PORT_B, .pin = 1 },
};

TASK(smoke_adc_task, TASK_MIN_STACK_SIZE) {
   for (uint8_t i = 0; i < SIZEOF_ARRAY(adc_addy); i++) {
      gpio_init_pin(&adc_addy[i], GPIO_ANALOG, GPIO_STATE_LOW);
      adc_add_channel(adc_addy[i]);
   }


   while(true) {
      uint16_t data = 0;
      for (uint8_t i = 0; i < SIZEOF_ARRAY(adc_addy); i++) {
        adc_read_converted(adc_addy[i], &data);
        LOG_DEBUG("%d%d: %d\n", adc_addy[i].port, adc_addy[i].pin, data);
      }
      delay_ms(1000);
   }
}

int main() {
   tasks_init();
   interrupt_init();
   gpio_init();
   log_init();
   adc_init(ADC_MODE_SINGLE);
   LOG_DEBUG("Welcome to ADC SMOKE TEST!");

   tasks_init_task(smoke_adc_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   return 0;
}
