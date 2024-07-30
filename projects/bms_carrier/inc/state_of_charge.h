#pragma once

#include "bms.h"
#include "current_sense.h"
#include "status.h"

#define PACK_CAPACITY_AH 124  // Not actually
#define OCV_TABLE_SIZE 20

typedef struct {
  uint32_t last_time;
  float i_soc;
  float v_soc;
  float averaged_soc;
} StateOfChargeStorage;

StatusCode state_of_charge_init(BmsStorage *bms_storage);

StatusCode update_state_of_chrage();
