#pragma once

#include "bms.h"
#include "current_sense.h"
#include "status.h"

#define PACK_CAPACITY_AH 124  // Not actually
#define PACK_INTERNAL_RESISTANCE_R 124 // Not actually

#define BATT_MODEL_C1 123 // Not actually. Should be in milliseconds
#define BATT_MODEL_C2 122 // Not actually. Should be in milliseconds

#define OCV_TABLE_SIZE 20

typedef struct {
  uint32_t last_time;
  float i_soc;
  float v_soc;
  float averaged_soc;

  float prev_rc1_voltage;
  float prev_rc2_voltage;
} StateOfChargeStorage;

StatusCode state_of_charge_init(BmsStorage *bms_storage);

StatusCode update_state_of_chrage();
