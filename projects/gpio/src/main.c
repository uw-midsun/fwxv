#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "gpio.h"
#include "status.h"
#include "task.h"
#include "interrupt.h"
#include "gpio_it.h"

#define BTN_INT_EVENT 5

// GpioAddress structs
GpioAddress addr1 = {
  .port = GPIO_PORT_B,
  .pin = 5,
};

GpioAddress addr2 = {
  .port = GPIO_PORT_B,
  .pin = 4,
};

GpioAddress addr3 = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

GpioAddress addr4 = {
  .port = GPIO_PORT_A,
  .pin = 15,
};

GpioAddress buttonAddr = {
  .port = GPIO_PORT_A,
  .pin = 7,
};

InterruptSettings settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .edge = INTERRUPT_EDGE_RISING,
};

TASK(gpio_toggle, TASK_STACK_512) {
  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH); 
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW); 
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH); 
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW); 
  gpio_init_pin(&buttonAddr, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

  uint32_t notification = 0;
  bool check;

  while (true) {

    notify_get(&notification); 
    check = notify_check_event(&notification, BTN_INT_EVENT);
    
    if (check) {
      gpio_toggle_state(&buttonAddr);
    }


    gpio_toggle_state(&addr1);
    delay_ms(1000);
    gpio_toggle_state(&addr2);
    delay_ms(1000);
    gpio_toggle_state(&addr3);
    delay_ms(1000);
    gpio_toggle_state(&addr4);
    delay_ms(1000);
  }
}



int main() {
    tasks_init();
    interrupt_init();
    gpio_init(); 

    gpio_it_register_interrupt(&buttonAddr, &settings, BTN_INT_EVENT, gpio_toggle);
    tasks_init_task(gpio_toggle, TASK_PRIORITY(1), NULL); 
    LOG_DEBUG("Program start...\n");
    tasks_start();
    
    return 0;
}
