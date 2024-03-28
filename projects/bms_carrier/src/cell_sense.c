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

  .num_devices = 1,
  .num_cells = 12,
  .num_thermistors = 12,
};

static inline StatusCode prv_cell_sense_conversions() {
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
    fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return status;
  }

  status |= ltc_afe_impl_read_cells(ltc_afe_storage);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("READ FAILED\n");
    fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return status;
  }

  // for (size_t i = 0; i < 10; i++) {
  //   status |= ltc_afe_impl_trigger_aux_conv(ltc_afe_storage, i);
  //   if (status != STATUS_CODE_OK) {
  //     fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
  //     return status;
  //   }
  //   delay_ms(CONV_DELAY_MS);
  // }
  return status;
}

// Task bc delays
TASK(cell_sense_conversions, TASK_STACK_256) {
  while (true) {
    notify_wait(NULL, BLOCK_INDEFINITELY);

    // run conversions every 10 seconds
    // if (xTaskGetTickCount() - ltc_afe_storage->timer_start >= pdMS_TO_TICKS(10000)) {
    prv_cell_sense_conversions();
    // }

    send_task_end();
  }
}

StatusCode cell_conversions() {
  StatusCode ret = notify(cell_sense_conversions, CELL_SENSE_CONVERSIONS);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("NOTIFY FAILED\n");
    fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

StatusCode cell_sense_run() {
  StatusCode status = STATUS_CODE_OK;
  uint16_t max_voltage = 0;
  uint16_t min_voltage = 0xffff;

  for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    // LOG_DEBUG("CELL %d: %d\n\r", cell,
    // ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]]);
    max_voltage =
        ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]] > max_voltage
            ? ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]]
            : max_voltage;
    min_voltage =
        ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]] < min_voltage
            ? ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]]
            : min_voltage;
    delay_ms(1);
  }
  LOG_DEBUG("MAX VOLTAGE: %d\n", max_voltage);
  LOG_DEBUG("MIN VOLTAGE: %d\n", min_voltage);
  set_battery_info_max_cell_v(max_voltage);

  if (max_voltage >= CELL_OVERVOLTAGE) {
    LOG_DEBUG("OVERVOLTAGE\n");
    fault_bps_set(BMS_FAULT_OVERVOLTAGE);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  if (min_voltage <= CELL_UNDERVOLTAGE) {
    LOG_DEBUG("UNDERVOLTAGE\n");
    fault_bps_set(BMS_FAULT_UNDERVOLTAGE);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  if (max_voltage - min_voltage >= CELL_UNBALANCED) {
    LOG_DEBUG("UNBALANCED\n");
    fault_bps_set(BMS_FAULT_UNBALANCE);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  delay_ms(1);
  if (min_voltage >= AFE_BALANCING_UPPER_THRESHOLD) {
    min_voltage += 20;
  } else if (min_voltage < AFE_BALANCING_UPPER_THRESHOLD &&
             min_voltage >= AFE_BALANCING_LOWER_THRESHOLD) {
    min_voltage += 100;
  } else {
    min_voltage += 250;
  }

  // if (xTaskGetTickCount() - ltc_afe_storage->timer_start >= 10000) {
  ltc_afe_storage->timer_start = xTaskGetTickCount();
  for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    if (ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]] > min_voltage) {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, true);
      // LOG_DEBUG("Cell %d unbalanced %d MIN VOLTAGE: %d\n", cell,
      // ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]], min_voltage);
    } else {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, false);
    }
    // }
  }

  // LOG_DEBUG("Config discharge bitset %d\n", ltc_afe_storage->discharge_bitset[0]);

  // for (size_t thermistor = 0;
  //      thermistor < (s_afe_settings.num_thermistors * s_afe_settings.num_devices); thermistor++)
  //      {
  //   ltc_afe_impl_read_aux(ltc_afe_storage, thermistor);

  //   // Log thermistor result
  //   LOG_DEBUG("Thermistor reading: %d\n",
  //             ltc_afe_storage->aux_voltages[ltc_afe_storage->aux_result_lookup[thermistor]]);

  //   if (ltc_afe_storage->aux_voltages[ltc_afe_storage->aux_result_lookup[thermistor]] >=
  //       CELL_MAX_TEMPERATURE) {
  //     fault_bps_set(BMS_FAULT_OVERTEMP_CELL);
  //     return STATUS_CODE_INTERNAL_ERROR;
  //   }
  // }

  return status;
}

StatusCode cell_sense_init(LtcAfeStorage *afe_storage) {
  ltc_afe_storage = afe_storage;
  ltc_afe_init(ltc_afe_storage, &s_afe_settings);
  delay_ms(10);
  tasks_init_task(cell_sense_conversions, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}
