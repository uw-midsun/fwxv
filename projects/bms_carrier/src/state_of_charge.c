#include "state_of_charge.h"

BmsStorage *bms_storage;
static StateOfChargeStorage s_storage;
static float ocv_voltage_weight = 0.75f;

static uint16_t ocv_voltage_lookup[OCV_TABLE_SIZE];

static void coulomb_counting_soc() {
  uint32_t d_time = pdTICKS_TO_MS(xTaskGetTickCount()) - s_storage.last_time;
  s_storage.last_time = pdTICKS_TO_MS(xTaskGetTickCount());

  float integrated_current = (float)(bms_storage->current_storage.current) * (d_time) /
                             (1000.0f * 3600.0f);  // in milliamp hours
  s_storage.i_soc = s_storage.averaged_soc - (integrated_current) / PACK_CAPACITY_MAH;

  if (s_storage.i_soc > 1.0f) {
    s_storage.i_soc = 1.0f;
  } else if (s_storage.i_soc < 0.0f) {
    s_storage.i_soc = 0.0f;
  }
}

static void ocv_voltage_soc() {
  // TO BE COMPLETED AFTER OCV LOOKUP TABLE IS MADE
  uint8_t low_index = 0xff;
  uint8_t upper_index = 0;

  for (uint8_t i = 0; i < OCV_TABLE_SIZE - 1; i++) {
    if (bms_storage->current_storage.voltage >= ocv_voltage_lookup[i] &&
        bms_storage->current_storage.voltage <= ocv_voltage_lookup[i + 1]) {
      low_index = i;
      upper_index = i + 1;
      break;
    }
  }

  if (low_index == 0xff || upper_index == 0) {
    return;
  }

  uint16_t voltage_low = ocv_voltage_lookup[low_index];
  uint16_t voltage_high = ocv_voltage_lookup[upper_index];

  // Lookup table index = SOC, voltage = value
  s_storage.v_soc = low_index + ((float)(upper_index - low_index) *
                                 (bms_storage->current_storage.voltage - voltage_low) /
                                 (voltage_high - voltage_low));

  // For now, lets assume we measure SOC over 5% increments
  // This means SOC = 5% * lookup_index
  s_storage.v_soc *= 5.0f;
}

StatusCode update_state_of_chrage() {
  coulomb_counting_soc();
  ocv_voltage_soc();

  s_storage.averaged_soc =
      (ocv_voltage_weight * s_storage.v_soc) + ((1 - ocv_voltage_weight) * (s_storage.i_soc));
  return STATUS_CODE_OK;
}

StatusCode state_of_charge_init(BmsStorage *bms_store) {
  bms_storage = bms_store;
  s_storage.last_time = 0;
  // No current sense voltage until all relays are closed, which means there is a load
  // Maybe use afe voltages???
  ocv_voltage_soc();
  s_storage.i_soc = s_storage.v_soc;
  s_storage.averaged_soc = s_storage.v_soc;

  if (ocv_voltage_weight > 1.0f) {
    ocv_voltage_weight = 1.0f;
  } else if (ocv_voltage_weight < 0.0f) {
    ocv_voltage_weight = 0.0f;
  }

  return STATUS_CODE_OK;
}
