#include "cell_sense.h"

LtcAfeStorage *ltc_afe_storage;

static const LtcAfeSettings s_afe_settings = {
  .mosi = AFE_SPI_MOSI,
  .miso = AFE_SPI_MISO,
  .sclk = AFE_SPI_SCK,
  .cs = AFE_SPI_CS,

  .spi_port = AFE_SPI_PORT,
  .spi_baudrate = 750000,

  .adc_mode = LTC_AFE_ADC_MODE_7KHZ,

  .cell_bitset = { 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF },
  .aux_bitset = { 0 },

  .num_devices = 2,
  .num_cells = 12,
  .num_thermistors = 12,
};

StatusCode cell_sense_conversions() {
  StatusCode status = STATUS_CODE_OK;
  // TODO: Figure out why cell_conv cannot happen without spi timing out (Most likely RTOS
  // implemntation error) Retry Mechanism
  if (ltc_afe_impl_trigger_cell_conv(ltc_afe_storage)) {
    // If this has failed, try once more after a short delay
    delay_ms(RETRY_DELAY_MS);
    status |= ltc_afe_impl_trigger_cell_conv(ltc_afe_storage);
  }
  delay_ms(CONV_DELAY_MS);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("CELL_SENSE (conv failed): %d\n", status);
    set_battery_status_fault(BMS_FAULT_COMMS_LOSS_AFE);
    set_battery_status_status(1);
    // fsm_transition(fsm, RELAYS_FAULT);
    return status;
  }

  for (size_t i = 0; i < 10; i++) {
    ltc_afe_impl_trigger_aux_conv(ltc_afe_storage, i);
    delay_ms(CONV_DELAY_MS);
    ltc_afe_impl_read_aux(ltc_afe_storage, i);

    // Log thermistor result
    LOG_DEBUG("Thermistor reading: %d\n",
              ltc_afe_storage->aux_voltages[ltc_afe_storage->aux_result_lookup[i]]);
  }
  return status;
}

StatusCode cell_sense_run() {
  StatusCode status = STATUS_CODE_OK;
  uint16_t max_voltage = 0;
  uint16_t min_voltage = 0xffff;

  status |= ltc_afe_impl_read_cells(ltc_afe_storage);
  for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    LOG_DEBUG("CELL %d: %d\n\r", cell,
              ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]]);
    max_voltage =
        ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]] > max_voltage
            ? ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]]
            : max_voltage;
    min_voltage =
        ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]] < min_voltage
            ? ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]]
            : min_voltage;
    delay_ms(2);
  }
  set_battery_vt_voltage(max_voltage);
  LOG_DEBUG("MAX VOLTAGE: %d\n", max_voltage);
  LOG_DEBUG("MIN VOLTAGE: %d\n", min_voltage);
  delay_ms(1);

  if (max_voltage >= OVERVOLTAGE_THRESHOLD) {
    LOG_DEBUG("OVERVOLTAGE\n");
    // fsm_transition(fsm, RELAYS_FAULT);
    set_battery_status_fault(BMS_FAULT_OVERVOLTAGE);
    set_battery_status_status(2);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  if (min_voltage <= UNDERVOLTAGE_THRESHOLD) {
    LOG_DEBUG("UNDERVOLTAGE\n");
    // fsm_transition(fsm, RELAYS_FAULT);
    set_battery_status_fault(BMS_FAULT_UNDERVOLTAGE);
    set_battery_status_status(1);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  if (max_voltage - min_voltage >= AFE_UNBALANCE_THRESHOLD) {
    LOG_DEBUG("UNBALANCED\n");
    // fsm_transition(fsm, RELAYS_FAULT);
    set_battery_status_fault(BMS_FAULT_UNBALANCE);
    set_battery_status_status(1);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (min_voltage >= AFE_BALANCING_UPPER_THRESHOLD) {
    min_voltage += 20;
  } else if (min_voltage < AFE_BALANCING_UPPER_THRESHOLD &&
             min_voltage >= AFE_BALANCING_LOWER_THRESHOLD) {
    min_voltage += 100;
  } else {
    min_voltage += 250;
  }

  for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    if (ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]] > min_voltage) {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, true);
      LOG_DEBUG("Cell %d unbalanced \n", cell);
      // TODO: add fault for BMS_FAULT_UNBALANCE
    } else {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, false);
    }
  }
  return status;
}

StatusCode cell_sense_init(LtcAfeStorage *afe_storage) {
  ltc_afe_storage = afe_storage;
  ltc_afe_init(ltc_afe_storage, &s_afe_settings);
  delay_ms(10);
  return STATUS_CODE_OK;
}
