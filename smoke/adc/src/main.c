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

static const GpioAddress tmp = 
  { .port = NUM_GPIO_PORTS, .pin = ADC_Channel_Vrefint };

TASK(smoke_adc_task, TASK_STACK_1024) {
   for (uint8_t i = 0; i < 1; i++) {
      gpio_init_pin(&adc_addy[i], GPIO_ANALOG, GPIO_STATE_LOW);
   }
   //adc_add_channel(tmp);
   adc_add_channel(adc_addy[0]);
   adc_init(ADC_MODE_SINGLE);


   while(true) {
      uint16_t data = 0;
      //for (uint8_t i = 0; i < SIZEOF_ARRAY(adc_addy); i++) {
        //adc_read_raw(tmp, &data);
        LOG_DEBUG("voltage ref: %d\n\r", data);
        adc_read_raw(adc_addy[0], &data);
        LOG_DEBUG("pinA0: %d\n\r", data);
      //}
      delay_ms(1000);
   }
}

int main() {
   tasks_init();
   interrupt_init();
   gpio_init();
   log_init();
   LOG_DEBUG("Welcome to ADC SMOKE TEST!\n\r");

   tasks_init_task(smoke_adc_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   return 0;
}
