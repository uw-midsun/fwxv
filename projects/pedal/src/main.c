#include <stdio.h>

#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "adc.h"
//#include "event_queue.h"
#include "can.h"
#include "can_msg.h"
#include "can_board_ids.h"
#include "log.h"
//#include "new_can_setters.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
   .device_id = 0x1,
   .bitrate = CAN_HW_BITRATE_500KBPS,
   .tx = { GPIO_PORT_A, 12 },
   .rx = { GPIO_PORT_A, 11 },
   .loopback = true,
};

// Initialize the GPIOs needed for the throttle
// Initialize ADC for ADC readings
// Calibrate the pedal upon initialization (set what we consider max value and min value for each pedal)
void init_pedal_controls() { 
   gpio_init();
   interrupt_init();
   soft_timer_init();
   adc_init(ADC_MODE_SINGLE);

   pedal_calibrate();
}

// Read any GPIOs and the ADC for throttle
bool read_pedal_throttle() {

}

// Read any GPIOs and the ADC for brake
bool read_pedal_brake() {

}

// Read the ADC when the throttle and brake are pressed down/let go, and use that reading to set the upper and lower bounds of the pedal
void pedal_calibrate() {

}

void run_fast_cycle()
{

}


// Should read whether the brake is pressed, and if so, disable the throttle
// 3 Cases when we transmit data:
   // 1. Throttle is pressed, brake is not pressed (pedal not pressed) - send throttle data as normal
   // 2. Throttle is pressed, brake is pressed (pedal pressed) - send throttle as 0
   // 3. Brake pressed - send throttle as 0


// const CanMessage can_msg = {
//    .id = 0x1,
//    .type = CAN_MSG_TYPE_DATA, // uint32_t?
//    .data = throttle_output(), // ?
//    // .target = motor_interface
// };
void run_medium_cycle()
{
   run_can_rx_cycle();
   wait_tasks(1);

   run_can_tx_cycle();
   wait_tasks(1);

   // Runs pedal_controls
   // Runs can_tx_task
   init_pedal_controls();
   tasks_init(CAN_TX, TASK_PRIORITY(1), NULL);


   //  if (read_pedal_throttle() && !read_pedal_brake()) {
   //     // Send throttle data as normal
   //     can_transmit(can_msg);
   //  } else if (read_pedal_throttle() && read_pedal_brake()) {
   //     // Send throttle as 0
   //     can_transmit(NULL);
   //  } else if (read_pedal_brake()) {
   //     // Send throttle as 0
   //     can_transmit(NULL);
   //  }
}

void run_slow_cycle()
{

}

TASK(master_task, TASK_MIN_STACK_SIZE) {
   int counter = 0;
   while (true) {
#ifdef TEST
      xSemaphoreTake(test_cycle_start_sem);
#endif
      run_fast_cycle();
      if (!(counter%10)) run_medium_cycle();
      if (!(counter%100)) run_slow_cycle();

#ifdef TEST
      xSemaphoreGive(test_cycle_end_sem);
#endif
      vTaskDelay(pdMS_TO_TICKS(100));
      ++counter;
    }
}

int main() {
   tasks_init();
   log_init();

   LOG_DEBUG("Welcome to CAN!");
   can_init(&s_can_storage, &can_settings);
   can_add_filter_in(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1);
   
   tasks_init_task(master_task, TASK_PRIORITY(2), NULL);
   
   tasks_start();
   
   LOG_DEBUG("exiting main?\n");
   return 0;
}

