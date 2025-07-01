#include "pedal.h"

#include "centre_console_setters.h"

static const GpioAddress brake = BRAKE_LIMIT_SWITCH;
static const GpioAddress throttle = ADC_HALL_SENSOR;

static PedalCalibBlob *s_calib_blob;

static void prv_read_throttle_data(uint32_t *reading) {
  volatile uint16_t adc_reading;
  adc_read_raw(throttle, &adc_reading);

  // Convert ADC Reading to readable voltage by normalizing with calibration data and dividing
  // to get percentage press. Brake is now just a GPIO. Negatives and > 100 values will be
  // capped.
  // int16_t range_throttle =
  //     s_calib_blob->throttle_calib.upper_value - s_calib_blob->throttle_calib.lower_value;
  int16_t range_throttle = 800;
  volatile float calculated_reading =
      (((float)adc_reading - (float)800.0f) / range_throttle);
  // Readings are inverted
  calculated_reading = 1 - calculated_reading;
  if (calculated_reading < 0) {
    calculated_reading = 0;
  } else if (calculated_reading > 1) {
    calculated_reading = 1;
  }
  // LOG_DEBUG("READING %d\n", (int)(calculated_reading * 100));
  memcpy(reading, &calculated_reading, sizeof(calculated_reading));
}

void pedal_run() {
  GpioState brake_state = 0;
  uint32_t throttle_position = 0;
  gpio_get_state(&brake, &brake_state);
  prv_read_throttle_data(&throttle_position);

  // Sending messages
  if (brake_state == GPIO_STATE_LOW) {
    // Brake is pressed - Send brake data with throttle as 1
    set_cc_pedal_brake_output(0);
    set_cc_pedal_throttle_output(throttle_position);
  } else {
    // Brake is not pressed
    set_cc_pedal_brake_output(1);
    set_cc_pedal_throttle_output(0);
  }
}

StatusCode pedal_init(PedalCalibBlob *calib_blob) {
  // Set pointers to static variables that will be provided to all pedal files
  interrupt_init();
  gpio_init_pin(&brake, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&throttle, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(throttle);
  s_calib_blob = calib_blob;
  return STATUS_CODE_OK;
}
