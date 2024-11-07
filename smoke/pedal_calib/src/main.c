#include "adc.h"
#include "calib.h"
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "log.h"
#include "pedal.h"
#include "pedal_calib.h"
#include "test_helpers.h"
#include "unity.h"

// ORIGINAL PEDAL BOARD:
// #define BRAKE_LIMIT_SWITCH
//   { .port = GPIO_PORT_B, .pin = 1 }

// #define ADC_HALL_SENSOR
//   { .port = GPIO_PORT_A, .pin = 1 }

// CENTRE CONSOLE PEDAL:
// #define BRAKE_LIMIT_SWITCH
//   { .port = GPIO_PORT_B, .pin = 13 }

// #define ADC_HALL_SENSOR
//   { .port = GPIO_PORT_A, .pin = 0 }

static const GpioAddress brake = BRAKE_LIMIT_SWITCH;
static const GpioAddress throttle = ADC_HALL_SENSOR;

PedalCalibBlob global_calib_blob = { 0 };
static PedalCalibrationStorage s_throttle_calibration_storage;

void test_throttle_calibration_run(void) {
  LOG_DEBUG("Please press and hold the throttle\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_throttle_calibration_storage, &global_calib_blob.throttle_calib,
                     PEDAL_PRESSED, &throttle);
  LOG_DEBUG("Completed sampling\n");

  adc_run();

  LOG_DEBUG("Please ensure the throttle is not being pressed.\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_throttle_calibration_storage, &global_calib_blob.throttle_calib,
                     PEDAL_UNPRESSED, &throttle);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("UPPER THRESHOLD: %d\n", global_calib_blob.throttle_calib.upper_value);
  LOG_DEBUG("LOWER THRESHOLD: %d\n", global_calib_blob.throttle_calib.lower_value);
  calib_commit();
}

TASK(smoke_pedal_calib_task, TASK_STACK_512) {
  gpio_init_pin(&brake, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&throttle, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(throttle);

  adc_init();
  adc_run();

  StatusCode ret = calib_init(&global_calib_blob, sizeof(global_calib_blob), true);
  if (ret == STATUS_CODE_OK) {
    LOG_DEBUG("calib_init test: OK\n");
  } else {
    LOG_DEBUG("calib_init test: FAILED (Error code: %d)\n", (int)ret);
  }

  pedal_calib_init(&s_throttle_calibration_storage);

  test_throttle_calibration_run();

  while (1) {
  }
}

int main() {
  tasks_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  flash_init();
  log_init();

  tasks_init_task(smoke_pedal_calib_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  return 0;
}
