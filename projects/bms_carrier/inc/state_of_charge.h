#pragma once

#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "status.h"

#define CELL_INTERNAL_RESISTANCE_mOHMS 22
#define PACK_INTERNAL_RESISTANCE_mOHMS \
  (CELL_INTERNAL_RESISTANCE_mOHMS * 9 * 4) / 8  // 9 modules, each module is 8P4S
#define VOLTS_TO_mV 1000
#define OCV_TABLE_SIZE 41

typedef struct {
  uint32_t last_time;
  float i_soc;
  float v_soc;
  float averaged_soc;

  int32_t last_current;
} StateOfChargeStorage;

void coulomb_counting_soc();
float perdict_ocv_voltage();
void ocv_voltage_soc();

StatusCode state_of_charge_init(BmsStorage *bms_storage);

StatusCode update_state_of_chrage();

// TEST FUNCTIONS
void ramp_voltage_weight();

void set_last_time(uint32_t last_time);
void set_i_soc(float i_soc);
void set_v_soc(float v_soc);
void set_averaged_soc(float averaged_soc);
void set_last_current(int32_t last_current);

uint32_t get_last_time(void);
float get_i_soc(void);
float get_v_soc(void);
float get_averaged_soc(void);
int32_t get_last_current(void);
float get_voltage_weight(void);
