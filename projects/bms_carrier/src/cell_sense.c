#include "cell_sense.h"

static LtcAfeStorage *ltc_afe_storage;
static BmsStorage *bms;

#define NUM_AFE_MSGS 4U
#define READINGS_PER_AFE_MSG 3U

#define TEMP_RESISTANCE 10000
#define VREF2 30000.0f                      // in 100uV
#define ADC_GAIN (VREF2 / ((1 << 15) - 1))  // 100uV / sample
#define TABLE_SIZE 125
#define LTC_RETRIES 5

#define CELL_VOLTAGE_LOOKUP(cell) \
  (ltc_afe_storage->cell_voltages[ltc_afe_storage->cell_result_lookup[cell]])

#define RETRY_OPERATION(max_retries, delay_ms_val, operation, status_var) \
  do {                                                                    \
    uint8_t _retries_left = (max_retries);                                \
    (status_var) = STATUS_CODE_INTERNAL_ERROR;                            \
    while (_retries_left-- > 0) {                                         \
      (status_var) = (operation);                                         \
      if ((status_var) == STATUS_CODE_OK) break;                          \
      delay_ms(delay_ms_val);                                             \
    }                                                                     \
  } while (0)

#define CELL_TEMP_OUTLIER 80
uint8_t afe_message_index = 0;
uint8_t failure_count = 0;

static bool prv_cell_data_updated = false;

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
  if (thermistor >= VREF2) return 0;

  thermistor = (uint16_t)(thermistor * ADC_GAIN);                                          // 100uV
  uint16_t thermistor_resistance = (thermistor * TEMP_RESISTANCE) / (VREF2 - thermistor);  // Ohms
  delay_ms(10);
  uint16_t min_diff = abs(thermistor_resistance - s_resistance_lookup[0]);

  uint8_t lo = 0;
  uint8_t hi = TABLE_SIZE - 1U;
  uint8_t best_index = 0;
  uint16_t best_diff = abs(s_resistance_lookup[0] - thermistor_resistance);

  /* Binary search */
  while (lo <= hi) {
    uint8_t mid = (lo + hi) / 2;
    uint16_t diff = abs(s_resistance_lookup[mid] - thermistor_resistance);

    if (diff < best_diff) {
      best_diff = diff;
      best_index = mid;
    }

    if (s_resistance_lookup[mid] > thermistor_resistance) {
      lo = mid + 1;  // Go right
    } else if (s_resistance_lookup[mid] < thermistor_resistance) {
      hi = mid - 1;  // Go left
    } else {
      return mid;  // Exact match
    }
  }

  return best_index;
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
  .aux_bitset = { 0x14, 0x15, 0x15 },

  .num_devices = 3,
  .num_cells = 12,
  .num_thermistors = NUM_THERMISTORS,
};

static StatusCode prv_cell_sense_conversions() {
  volatile StatusCode status = STATUS_CODE_OK;
  // TODO: Figure out why cell_conv cannot happen without spi timing out (Most likely RTOS
  // implemntation error) Retry Mechanism

  RETRY_OPERATION(LTC_RETRIES, RETRY_DELAY_MS, ltc_afe_impl_trigger_cell_conv(ltc_afe_storage),
                  status);

  if (status) {
    // If this has failed, try once more after a short delay
    LOG_DEBUG("Cell trigger conv failed, retrying): %d\n", status);
    delay_ms(RETRY_DELAY_MS);
    status = ltc_afe_impl_trigger_cell_conv(ltc_afe_storage);
  }

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell conv failed): %d\n", status);

    if (++failure_count >= CELL_SENSE_MAX_FAILURES) {
      fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    }

    return status;
  }
  delay_ms(CONV_DELAY_MS);

  RETRY_OPERATION(LTC_RETRIES, RETRY_DELAY_MS, ltc_afe_impl_read_cells(ltc_afe_storage), status);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell read failed %d\n", status);

    if ((status == STATUS_CODE_UNREACHABLE) || (status == STATUS_CODE_TIMEOUT)) {
      /* Lost comms */
      fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    } else if (++failure_count >= CELL_SENSE_MAX_FAILURES) {
      fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    }

    return status;
  }

  for (size_t thermistor = 0; thermistor < s_afe_settings.num_thermistors; thermistor += 1) {
    // Thermistor indexes are read for each daisy chained dev at the same time
    // Check therm bitset to determine if we need to read any at this index
    bool check_therm = false;
    for (uint8_t dev = 0; dev < s_afe_settings.num_devices; dev++) {
      if ((s_afe_settings.aux_bitset[dev] >> thermistor) & 0x1) {
        check_therm = true;
      }
    }

    if (check_therm) {
      // Trigger and read thermistor value
      RETRY_OPERATION(LTC_RETRIES, RETRY_DELAY_MS,
                      ltc_afe_impl_trigger_aux_conv(ltc_afe_storage, s_thermistor_map[thermistor]),
                      status);

      if (status) {
        LOG_DEBUG("Thermistor conv failed for therm %d: Status %d\n", (uint8_t)thermistor, status);

        if (++failure_count >= CELL_SENSE_MAX_FAILURES) {
          fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
        }
        return status;
      }
      delay_ms(AUX_CONV_DELAY_MS);

      RETRY_OPERATION(LTC_RETRIES, RETRY_DELAY_MS,
                      ltc_afe_impl_read_aux(ltc_afe_storage, thermistor), status);

      if (status) {
        LOG_DEBUG("Thermistor read trigger failed for thermistor %d,  %d\n", (uint8_t)thermistor,
                  status);

        if (status == STATUS_CODE_UNREACHABLE || status == STATUS_CODE_TIMEOUT) {
          /* Lost comms */
          fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
        }

        if (++failure_count >= CELL_SENSE_MAX_FAILURES) {
          fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
        }

        return status;
      }
    }
  }

  if (status == STATUS_CODE_OK) {
    // If we arrive here, there has been no failed readings
    failure_count = 0;
  }

  return status;
}

static StatusCode prv_cell_sense_run() {
  StatusCode status = STATUS_CODE_OK;
  uint16_t max_voltage = 0;
  uint16_t min_voltage = 0xffff;
  for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    uint16_t voltage = CELL_VOLTAGE_LOOKUP(cell);
    LOG_DEBUG("CELL %d: %d\n\r", (uint8_t)cell, voltage);
    delay_ms(5);

    max_voltage = voltage > max_voltage ? voltage : max_voltage;
    min_voltage = voltage < min_voltage ? voltage : min_voltage;
  }

  // delay_ms(10);
  // LOG_DEBUG("MAX VOLTAGE: %d\n", max_voltage);
  // delay_ms(10);
  // LOG_DEBUG("MIN VOLTAGE: %d\n", min_voltage);
  // delay_ms(10);
  // LOG_DEBUG("UNBALANCE: %d\n", max_voltage - min_voltage);
  set_battery_info_max_cell_v(max_voltage);
  set_battery_info_min_cell_v(min_voltage);

  if (max_voltage >= SOLAR_VOLTAGE_THRESHOLD) {
    bms_open_solar();
  }

  if (max_voltage >= CELL_OVERVOLTAGE) {
    LOG_DEBUG("OVERVOLTAGE\n");
    fault_bps_set(BMS_FAULT_OVERVOLTAGE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }
  if (min_voltage <= CELL_UNDERVOLTAGE) {
    LOG_DEBUG("UNDERVOLTAGE\n");
    fault_bps_set(BMS_FAULT_UNDERVOLTAGE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }
  if (max_voltage - min_voltage >= CELL_UNBALANCED) {
    LOG_DEBUG("UNBALANCED\n");

    if (++failure_count >= CELL_SENSE_MAX_FAILURES) {
      fault_bps_set(BMS_FAULT_UNBALANCE);
    }

    status = STATUS_CODE_INTERNAL_ERROR;
  }

  if (min_voltage >= AFE_BALANCING_UPPER_THRESHOLD) {
    min_voltage += 20;
  } else if (min_voltage < AFE_BALANCING_UPPER_THRESHOLD &&
             min_voltage >= AFE_BALANCING_LOWER_THRESHOLD) {
    min_voltage += 100;
  } else {
    min_voltage += 100;
  }

  // Balancing
  for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    if (CELL_VOLTAGE_LOOKUP(cell) > min_voltage) {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, true);
    } else {
      ltc_afe_impl_toggle_cell_discharge(ltc_afe_storage, cell, false);
    }
  }

  ltc_afe_impl_write_config(ltc_afe_storage);

  // Log and check all thermistor values based on settings bitset
  uint16_t max_temp = 0;
  for (uint8_t dev = 0; dev < s_afe_settings.num_devices; dev++) {
    for (uint8_t thermistor = 0; thermistor < LTC_AFE_MAX_THERMISTORS_PER_DEVICE; thermistor += 1) {
      if ((s_afe_settings.aux_bitset[dev] >> thermistor) & 0x1) {
        uint8_t index = dev * LTC_AFE_MAX_THERMISTORS_PER_DEVICE + thermistor;
        uint16_t temp = calculate_temperature(ltc_afe_storage->aux_voltages[index]);
        ltc_afe_storage->aux_voltages[index] = temp;
        // LOG_DEBUG("Thermistor reading dev %d, %d: %d\n", dev, thermistor,
        //           ltc_afe_storage->aux_voltages[index]);
        // delay_ms(3);
        if (ltc_afe_storage->aux_voltages[index] > CELL_TEMP_OUTLIER) {
          continue;
        }

        max_temp = temp > max_temp ? temp : max_temp;

        bool is_discharging = bms->pack_current < 0;
        uint16_t limit =
            is_discharging ? CELL_MAX_TEMPERATURE_DISCHARGE : CELL_MAX_TEMPERATURE_CHARGE;

        if (temp >= limit) {
          LOG_DEBUG("CELL OVERTEMP\n");
          fault_bps_set(BMS_FAULT_OVERTEMP_CELL);

          if (++failure_count >= CELL_SENSE_MAX_FAILURES) {
            fault_bps_set(BMS_FAULT_OVERTEMP_CELL);
          }

          status = STATUS_CODE_INTERNAL_ERROR;
        }
      }
    }
  }
  ltc_afe_storage->max_temp = max_temp;

  prv_cell_data_updated = true;

  if (status == STATUS_CODE_OK) {
    // If we arrive here, there has been no failed readings
    failure_count = 0;
  }

  return status;
}

StatusCode log_cell_sense() {
  // CAN Logging
  // AFE messages are logged with 3 voltages at a time, and an index 0-3 to encompass all voltages
  if (prv_cell_data_updated != true) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  const uint8_t NUM_MSG = 4;
  const uint8_t READINGS_PER_MSG = 3;

  uint8_t read_index = afe_message_index * READINGS_PER_AFE_MSG;
  set_AFE1_status_id(afe_message_index);
  set_AFE1_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  set_AFE1_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  set_AFE1_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  read_index = (uint8_t)s_afe_settings.num_cells + afe_message_index * READINGS_PER_AFE_MSG;
  set_AFE2_status_id(afe_message_index);
  set_AFE2_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  set_AFE2_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  set_AFE2_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  read_index = (uint8_t)s_afe_settings.num_cells * 2 + afe_message_index * READINGS_PER_AFE_MSG;
  set_AFE3_status_id(afe_message_index);
  set_AFE3_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  set_AFE3_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  set_AFE3_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  // Thermistors to send are at index 0, 2, 4 for each device
  if (afe_message_index <
      NUM_AFE_MSGS - 1) {  // Only 3 thermistors per device, so 4th message will be ignored
    set_AFE1_status_temp(ltc_afe_storage->aux_voltages[afe_message_index * 2]);
    set_AFE2_status_temp(
        ltc_afe_storage->aux_voltages[LTC_AFE_MAX_THERMISTORS_PER_DEVICE + afe_message_index * 2]);
    set_AFE3_status_temp(ltc_afe_storage->aux_voltages[LTC_AFE_MAX_THERMISTORS_PER_DEVICE * 2 +
                                                       afe_message_index * 2]);
  }

  afe_message_index = (afe_message_index + 1) % NUM_MSG;

  if (afe_message_index == 0) {
    prv_cell_data_updated = false;
  }

  return STATUS_CODE_OK;
}

// Task bc delays
TASK(cell_sense_conversions, TASK_STACK_512) {
  ltc_afe_init(ltc_afe_storage, &s_afe_settings);
  delay_ms(10);
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (true) {
    if (prv_cell_sense_conversions() == STATUS_CODE_OK) {
      prv_cell_sense_run();
    }

    /* Delay is TBD */
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
  }
}

StatusCode cell_sense_init(BmsStorage *storage) {
  bms = storage;
  ltc_afe_storage = &(bms->ltc_afe_storage);
  tasks_init_task(cell_sense_conversions, TASK_PRIORITY(4), NULL);
  return STATUS_CODE_OK;
}
