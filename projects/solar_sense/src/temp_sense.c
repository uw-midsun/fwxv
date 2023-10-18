#include "temp_sense.h"

GpioState curr_state;

static GpioAddress temp_sense_pins[NUM_TEMP_SENSE_PINS] = {
  [TEMP_SENSE0] = TEMP_SENSE_GPIO0,
  [TEMP_SENSE1] = TEMP_SENSE_GPIO1,
  [TEMP_SENSE2] = TEMP_SENSE_GPIO2,
  [TEMP_SENSE3] = TEMP_SENSE_GPIO3,
  [TEMP_SENSE4] = TEMP_SENSE_GPIO4,
  [TEMP_SENSE5] = TEMP_SENSE_GPIO5,
  [TEMP_SENSE_OVERTEMP] = TEMP_SENSE_OVERTEMP_GPIO,
  [TEMP_SENSE_FULLSPEED] = TEMP_SENSE_FULLSPEED_GPIO,
  [TEMP_SENSE_FANFAIL] = TEMP_SENSE_FANFAIL_GPIO,
};

// Calculates the temperature based on the analog input for A0 and A1
uint16_t prv_calculateTempDigital1(uint16_t voltage_reading) {
  if (voltage_reading > MAX_VOLTS1) {
    voltage_reading = MAX_VOLTS1;
  } else if (voltage_reading < MIN_VOLTS1) {
    voltage_reading = MIN_VOLTS1;
  }

  return MIN_TEMP +
         ((MAX_TEMP - MIN_TEMP) * ((voltage_reading - MIN_VOLTS1) / (MAX_VOLTS1 - MIN_VOLTS1)));
}

// Calculates the temperature based on the analog input for A2 - A5
uint16_t prv_calculateTempDigital2(uint16_t voltage_reading) {
  if (voltage_reading > MIN_VOLTS2) {
    voltage_reading = MIN_VOLTS2;
  } else if (voltage_reading < MAX_VOLTS2) {
    voltage_reading = MAX_VOLTS2;
  }

  return MIN_TEMP +
         ((MAX_TEMP - MIN_TEMP) * ((voltage_reading - MIN_VOLTS2) / (MAX_VOLTS2 - MIN_VOLTS2)));
}

TASK(temp_sense_task, TASK_STACK_512) {
  LOG_DEBUG("Test");
  int i;
  uint16_t temp_digital;
  uint16_t voltage_reading;

  while (true) {
    // Overtemp, Full speed, and Fan fail
    gpio_get_state(&(temp_sense_pins[TEMP_SENSE_OVERTEMP]), &curr_state);
    set_thermal_status_overtemp(curr_state);
    gpio_get_state(&(temp_sense_pins[TEMP_SENSE_FULLSPEED]), &curr_state);
    set_thermal_status_fullspeed(curr_state);
    gpio_get_state(&(temp_sense_pins[TEMP_SENSE_FANFAIL]), &curr_state);
    set_thermal_status_fan_fail(curr_state);

    // A0
    adc_read_converted(temp_sense_pins[0], &voltage_reading);
    temp_digital = calculateTempDigital1(voltage_reading);
    set_thermal_status_temp_1(temp_digital);

    // A1
    adc_read_converted(temp_sense_pins[1], &voltage_reading);
    temp_digital = calculateTempDigital1(voltage_reading);
    set_thermal_status_temp_2(temp_digital);

    // A2
    adc_read_converted(temp_sense_pins[2], &voltage_reading);
    temp_digital = calculateTempDigital2(voltage_reading);
    set_thermal_temps_temp_3(temp_digital);

    // A3
    adc_read_converted(temp_sense_pins[3], &voltage_reading);
    temp_digital = calculateTempDigital2(voltage_reading);
    set_thermal_temps_temp_4(temp_digital);

    // A4
    adc_read_converted(temp_sense_pins[4], &voltage_reading);
    temp_digital = calculateTempDigital2(voltage_reading);
    set_thermal_temps_temp_5(temp_digital);

    // A5
    adc_read_converted(temp_sense_pins[5], &voltage_reading);
    temp_digital = calculateTempDigital2(voltage_reading);
    set_thermal_temps_temp_6(temp_digital);
  }
}

StatusCode temp_sense_adc_init() {
  int p;
  for (p = 0; p < NUM_TEMP_SENSE_PINS; p++) {
    if (p < TEMP_SENSE_OVERTEMP) {
      gpio_init_pin(&(temp_sense_pins[p]), GPIO_ANALOG, GPIO_STATE_LOW);
      adc_add_channel(temp_sense_pins[p]);
    } else {
      gpio_init_pin(&temp_sense_pins[p], GPIO_INPUT_PULL_UP, GPIO_STATE_LOW);
    }
  }

  tasks_init_task(temp_sense_task, TASK_PRIORITY(2), NULL);

  return STATUS_CODE_OK;
}
