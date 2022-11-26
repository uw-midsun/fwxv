#include "func.h"
#include "gpio.h"
#include "tasks.h"
#include "delay.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

void ledtask(void * a) {
  while (1) {
    gpio_toggle_state(&leds[0]);
    delay_ms(500);
  }
}

static StackType_t ledstack[512];
static StaticTask_t tcb;

int main(void) {
  gpio_init_pin(&leds[0], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&leds[1], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&leds[2], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&leds[3], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  gpio_toggle_state(&leds[0]);
  
  if (xTaskCreateStatic(ledtask, "led_task", 512, NULL, 2, ledstack, &tcb) == NULL){
    gpio_toggle_state(&leds[0]);
  }
  vTaskStartScheduler();

  while(1){}
  
  return 0;
}
