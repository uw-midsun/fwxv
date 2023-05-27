#include "gpio.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "tasks.h"
#include "log.h"

static Event BTN_INT_EVENT = 5;
static uint32_t notification = 5;

// Task static entities
static GpioAddress led1 = {
   .port = GPIO_PORT_B,
   .pin = 5,
};
static GpioAddress led2 = {
   .port = GPIO_PORT_B,
   .pin = 4,
};
static GpioAddress led3 = {
   .port = GPIO_PORT_B,
   .pin = 3,
};
static GpioAddress led4 = {
   .port = GPIO_PORT_A,
   .pin = 15,
};
static GpioAddress button = {
   .port = GPIO_PORT_A,
   .pin = 7,
};
static InterruptSettings settings = {
   .type = INTERRUPT_TYPE_INTERRUPT,
   .priority = INTERRUPT_PRIORITY_NORMAL,
   .edge = INTERRUPT_EDGE_RISING,
};


TASK(gpio_task, TASK_STACK_512) {
   LOG_DEBUG("gpio_task initialized!\n");

   //gpio_it_trigger_interrupt(&button);
   notify_get(&notification);

   if (notify_check_event(&notification, BTN_INT_EVENT) == 1) {
      gpio_toggle_state(&led1);
      gpio_toggle_state(&led2);
      gpio_toggle_state(&led3);
      gpio_toggle_state(&led4);
   }  
}

int main(void) {
   gpio_init_pin(&led1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
   gpio_init_pin(&led2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
   gpio_init_pin(&led3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
   gpio_init_pin(&led4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

   gpio_init_pin(&button, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

   interrupt_init();
   gpio_init();
   gpio_it_register_interrupt(&button, &settings, BTN_INT_EVENT, gpio_task);
   log_init();
   tasks_init();

   tasks_init_task(gpio_task, TASK_PRIORITY(2), NULL);

   LOG_DEBUG("Program start...\n");
   tasks_start();

   return 0;
}
