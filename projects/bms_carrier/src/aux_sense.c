#include "aux_sense.h"

static const GpioAddress aux_sense_pin = { .port = GPIO_PORT_A, .pin = 5 };
BmsStorage *storage;

StatusCode aux_sense_init(BmsStorage *storage) {
  storage = storage;
  gpio_init_pin(&aux_sense_pin, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(aux_sense_pin);
  adc_init();

  return STATUS_CODE_OK;
}

StatusCode aux_sense_run() {
  adc_run();
  adc_read_converted(aux_sense_pin, &storage->aux_batt_voltage);
  storage->aux_batt_voltage =
      (storage->aux_batt_voltage) * (R2_OHMS + R1_OHMS) / (R2_OHMS * 100);  // CONV VOLTAGE
  LOG_DEBUG("AUX READING: %d\n", storage->aux_batt_voltage);
  set_battery_status_aux_batt_v(storage->aux_batt_voltage);
  return STATUS_CODE_OK;
}
