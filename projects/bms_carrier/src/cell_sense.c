#include "cell_sense.h"

LtcAfeStorage *ltc_afe_storage;

#define TEMP_RESISTANCE 10000
#define VREF2 30000
#define TABLE_SIZE 125

typedef enum ThermistorMap {
  THERMISTOR_2 = 0,
  THERMISTOR_1,
  THERMISTOR_0,
  THERMISTOR_3,
  THERMISTOR_4,
  THERMISTOR_7,
  THERMISTOR_5,
  THERMISTOR_6,
  NUM_THERMISTORS
} ThermistorMap;

static uint8_t s_thermistor_map[NUM_THERMISTORS] = {
  [0] = THERMISTOR_0, [1] = THERMISTOR_1, [2] = THERMISTOR_2, [3] = THERMISTOR_3,
  [4] = THERMISTOR_4, [5] = THERMISTOR_5, [6] = THERMISTOR_6, [7] = THERMISTOR_7
};

static const uint16_t s_resistance_lookup[TABLE_SIZE] = {
  27219, 26076, 24988, 23951, 22963, 22021, 21123, 20267, 19450, 18670, 17925, 17214, 16534, 15886,
  15266, 14674, 14173, 13718, 13256, 12805, 12394, 12081, 11628, 11195, 10780, 10000, 9634,  9283,
  8947,  8624,  8314,  8018,  7733,  7460,  7199,  6947,  6706,  6475,  6252,  6039,  5834,  5636,
  5445,  5262,  5093,  4927,  4763,  4601,  4446,  4300,  4161,  4026,  3896,  3771,  3651,  3535,
  3423,  3315,  3211,  3111,  3014,  2922,  2833,  2748,  2665,  2586,  2509,  2435,  2364,  2294,
  2227,  2162,  2101,  2040,  1981,  1925,  1868,  1817,  1765,  1716,  1668,  1622,  1577,  1533,
  1490,  1449,  1410,  1371,  1334,  1298,  1263,  1229,  1197,  1164,  1134,  1107,  1078,  1052,
  1025,  999,   973,   949,   925,   902,   880,   858,   837,   816,   796,   777,   758,   739,
  721,   704,   687,   671,   655,   640,   625,   610,   596,   582,   569,   556,   543
};

int calculate_temperature(uint16_t thermistor) {
  // INCOMPLETE
  uint16_t thermistor_resistance = (thermistor * TEMP_RESISTANCE) / (VREF2 - thermistor);
  uint16_t min_diff = abs(thermistor_resistance - s_resistance_lookup[0]);

  for (int i = 1; i < TABLE_SIZE; ++i) {
    if (abs(thermistor_resistance - s_resistance_lookup[i]) < min_diff) {
      min_diff = abs(thermistor_resistance - s_resistance_lookup[i]);
      thermistor = i;
    }
  }
  return thermistor;
}

static const LtcAfeSettings s_afe_settings = {
  .mosi = AFE_SPI_MOSI,
  .miso = AFE_SPI_MISO,
  .sclk = AFE_SPI_SCK,
  .cs = AFE_SPI_CS,

  .spi_port = AFE_SPI_PORT,
  .spi_baudrate = 750000,

  .adc_mode = LTC_AFE_ADC_MODE_7KHZ,

  .cell_bitset = { 0xFFF, 0xFFF, 0xFFF },
  .aux_bitset = { 0xFF, 0xFF, 0xFF },

  .num_devices = 1,
  .num_cells = 12,
  .num_thermistors = 6,
};

static inline StatusCode prv_cell_sense_conversions() {
  StatusCode status = STATUS_CODE_OK;
  // TODO: Figure out why cell_conv cannot happen without spi timing out (Most likely RTOS
  // implemntation error) Retry Mechanism
  if (ltc_afe_impl_trigger_cell_conv(ltc_afe_storage)) {
    // If this has failed, try once more after a short delay
    status |= ltc_afe_impl_trigger_cell_conv(ltc_afe_storage);
  }
  delay_ms(CONV_DELAY_MS);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell conv failed): %d\n", status);
    fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return status;
  }

  status |= ltc_afe_impl_read_cells(ltc_afe_storage);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell read failed\n");
    fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return status;
  }

  for (size_t i = 0; i < s_afe_settings.num_thermistors * s_afe_settings.num_devices; i += 2) {
    // Thermistor select cell 0
    status |= ltc_afe_impl_trigger_aux_conv(ltc_afe_storage, s_thermistor_map[i]);
    // DELAY NEEDED for adc conv to happen
    delay_ms(1);
    // Thermistor read cell 0
    status |= ltc_afe_impl_read_aux(ltc_afe_storage, i);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Thermistor read/conv failed\n");
      fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
      return status;
    }

    // Log thermistor result
    ltc_afe_storage->aux_voltages[ltc_afe_storage->aux_result_lookup[i]] =
        calculate_temperature(ltc_afe_storage->aux_voltages[ltc_afe_storage->aux_result_lookup[i]]);
  }

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

  if (min_voltage >= AFE_BALANCING_UPPER_THRESHOLD) {
    min_voltage += 20;
  } else if (min_voltage < AFE_BALANCING_UPPER_THRESHOLD &&
             min_voltage >= AFE_BALANCING_LOWER_THRESHOLD) {
    min_voltage += 100;
  } else {
    min_voltage += 250;
  }
  // ltc_afe_storage->timer_start = xTaskGetTickCount();
  for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    if (ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]] > min_voltage) {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, true);
    } else {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, false);
    }
  }

  LOG_DEBUG("Config discharge bitset %d\n", ltc_afe_storage->discharge_bitset[0]);

  for (size_t thermistor = 0;
       thermistor < s_afe_settings.num_thermistors * s_afe_settings.num_devices; thermistor += 2) {
    // Log thermistor result
    LOG_DEBUG("Thermistor reading: %d\n",
              ltc_afe_storage->aux_voltages[ltc_afe_storage->aux_result_lookup[thermistor]]);

    if (ltc_afe_storage->aux_result_lookup[thermistor] >= CELL_MAX_TEMPERATURE) {
      LOG_DEBUG("CELL OVERTEMP\n");
      fault_bps_set(BMS_FAULT_OVERTEMP_CELL);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }

  return status;
}

StatusCode cell_discharge(LtcAfeStorage *afe_storage) {
  return ltc_afe_impl_write_config(afe_storage);
}

StatusCode cell_sense_init(LtcAfeStorage *afe_storage) {
  ltc_afe_storage = afe_storage;
  ltc_afe_init(ltc_afe_storage, &s_afe_settings);
  delay_ms(10);
  tasks_init_task(cell_sense_conversions, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}
