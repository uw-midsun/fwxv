#include <stdio.h>
//#include "ads1015.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "soft_timer.h"
#include "adc.h"
//#include "event_queue.h"
#include "can.h"
#include "can_msg.h"
#include "can_board_ids.h"
#include "log.h"
#include "tasks.h"
// #include "pedal_setters.h"

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

// static Ads1015Storage s_ads1015_storage = { 0 };
// static PedalCalibBlob s_calib_blob = { 0 };

// Read any GPIOs and the ADC for throttle
bool read_pedal_throttle() {
   return false;
}

// Read any GPIOs and the ADC for brake
bool read_pedal_brake() {
   return false;
}

// Read the ADC when the throttle and brake are pressed down/let go, and use that reading to set the upper and lower bounds of the pedal
void pedal_calibrate() {
   return;
}

void run_fast_cycle()
{
   return;
}

// Initialize the GPIOs needed for the throttle
// Initialize ADC for ADC readings
// Calibrate the pedal upon initialization (set what we consider max value and min value for each pedal)
void init_pedal_controls() { 
   interrupt_init();
   gpio_init(); 
   gpio_it_init();

   // setup ADC readings
   I2CSettings i2c_settings = {
      .speed = I2C_SPEED_FAST,
      .scl = { .port = GPIO_PORT_B, .pin = 10 },
   .sda = { .port = GPIO_PORT_B, .pin = 11 },
   };
   i2c_init(I2C_PORT_2, &i2c_settings);
   GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };
   //ads1015_init(&s_ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
   adc_init(ADC_MODE_SINGLE);

   pedal_calibrate();
}

void run_medium_cycle()
{
   run_can_tx_cycle();
   wait_tasks(1);

   // msg1 - brake
   // msg2 - throttle
   if (read_pedal_brake()) {
      // Edit message to send 1 for brake, 0 for throttle (if brake is pressed, disable the throttle)
      // set_transmit_msg1_status(1);
      // set_transmit_msg2_signal(0);
   } else if (read_pedal_throttle()) {
      // Edit message to send 1 for throttle (send throttle data as normal)
      // set_transmit_msg1_status(0);
      // set_transmit_msg2_signal(1);
   } else {
      // Edit message to send 0 for both
      // set_transmit_msg1_status(0);
      // set_transmit_msg2_signal(0);
   }

    // transmit message: use pedal_can_setters.h file to set the correct data for the can message instead of using the struct
}

void run_slow_cycle()
{
   return;
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

   LOG_DEBUG("Welcome to CAN!\n");
   init_pedal_controls();
   can_init(&s_can_storage, &can_settings);
   can_add_filter_in(SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT);

   tasks_init_task(master_task, TASK_PRIORITY(2), NULL);
   tasks_start();
   LOG_DEBUG("exiting main?\n");
   return 0;
}

