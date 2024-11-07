#include "aux_sense.h"

static const GpioAddress aux_sense_pin = { .port = GPIO_PORT_A, .pin = 5 };
AuxStorage *s_aux_storage;

StatusCode aux_sense_init(AuxStorage *aux_storage) {
  s_aux_storage = aux_storage;
  gpio_init_pin(&aux_sense_pin, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(aux_sense_pin);
  adc_init();

  return STATUS_CODE_OK;
}

StatusCode aux_sense_run() {
  adc_run();
  adc_read_converted(aux_sense_pin, &s_aux_storage->aux_battery_voltage);
  s_aux_storage->aux_battery_voltage = (s_aux_storage->aux_battery_voltage) * (R2_OHMS + R1_OHMS) /
                                       (R2_OHMS * 100);  // CONV VOLTAGE;
  LOG_DEBUG("AUX READING: %d\n", s_aux_storage->aux_battery_voltage);
  set_battery_status_aux_batt_v(s_aux_storage->aux_battery_voltage);
  return STATUS_CODE_OK;
}
