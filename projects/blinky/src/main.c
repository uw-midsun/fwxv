#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "gpio.h"
#include "log.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "wait.h"
#include "delay.h"
#include "tasks.h"
#include "notify.h"

#define BTN_INT_EVENT 5

// Controller Board LEDs
static const GpioAddress leds[] = {
   { .port = GPIO_PORT_B, .pin = 5 },
   { .port = GPIO_PORT_B, .pin = 4 },
   { .port = GPIO_PORT_B, .pin = 3 },
   { .port = GPIO_PORT_B, .pin = 15 },
};

// Buttons
static const GpioAddress buttons[] = {
   { .port = GPIO_PORT_A, .pin = 7 },
};

// Interrupt Settings
static InterruptSettings interrupt_settings = {
   .type = INTERRUPT_TYPE_INTERRUPT,
   .priority = INTERRUPT_PRIORITY_NORMAL,
};

// Gpio Task
TASK(gpio_task, TASK_STACK_512) {
   gpio_init_pin(&leds[0], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
   gpio_init_pin(&leds[1], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
   gpio_init_pin(&leds[2], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
   gpio_init_pin(&leds[3], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
   gpio_init_pin(&buttons[0], GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

   uint32_t notification;
   Event event;

   while (true) {
      if (notify_get(&notification) == STATUS_CODE_OK) {              // Gets the notification value
         if (notify_check_event(&notification, BTN_INT_EVENT)) {      // Checks if the notification is equal to BTN_INT_EVENT 
            for (size_t i = 0; i <  SIZEOF_ARRAY(leds); i++) {
            gpio_toggle_state(&leds[i]);
            }
         }
      }
   }
}

int main() {
   // Enables
   interrupt_init();
   gpio_init();
   tasks_init();

   gpio_it_register_interrupt(&buttons[0], &interrupt_settings,
                              BTN_INT_EVENT, gpio_task);
   
   tasks_init_task(gpio_task, TASK_PRIORITY(1), NULL);

   tasks_start();

   while (true) {
      gpio_it_trigger_interrupt(&buttons[0]);
      delay_ms(1000);
   }

   return 0;
}

