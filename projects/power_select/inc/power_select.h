#pragma once

#include <stdio.h>

#include "adc.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "power_select_setters.h"

// POWER SELECT DEFINITIONS

typedef enum PowerSelectStateId {
  POWER_SELECT_INACTIVE = 0,
  POWER_SELECT_ACTIVE,
} PowerSelectStateId;

#define NUM_POWER_SELECT_STATES 2
#define NUM_POWER_SELECT_TRANSITIONS 2

// POWER SUPPLY DEFINITIONS

#define POWER_SELECT_PWR_SUP_VALID_ADDR \
  { GPIO_PORT_A, 1 }
#define POWER_SELECT_PWR_SUP_VSENSE_ADDR \
  { GPIO_PORT_A, 2 }
#define POWER_SELECT_PWR_SUP_ISENSE_ADDR \
  { GPIO_PORT_A, 3 }

#define POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_PWR_SUP_MAX_CURRENT_MA 37500

#define POWER_SELECT_PWR_SUP_STATUS_MASK 0x01
#define POWER_SELECT_PWR_SUP_FAULT_OC_MASK 0x01
#define POWER_SELECT_PWR_SUP_FAULT_OV_MASK 0x02

DECLARE_FSM(power_supply);

StatusCode init_power_supply(void);

// DCDC DEFINITIONS

#define POWER_SELECT_DCDC_VALID_ADDR \
  { GPIO_PORT_A, 4 }
#define POWER_SELECT_DCDC_VSENSE_ADDR \
  { GPIO_PORT_A, 5 }
#define POWER_SELECT_DCDC_ISENSE_ADDR \
  { GPIO_PORT_A, 6 }
#define POWER_SELECT_DCDC_TSENSE_ADDR \
  { GPIO_PORT_A, 7 }

#define POWER_SELECT_DCDC_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_DCDC_MAX_CURRENT_MA 37500
#define POWER_SELECT_DCDC_MAX_TEMP_C 1

#define POWER_SELECT_DCDC_STATUS_MASK 0x02
#define POWER_SELECT_DCDC_FAULT_OV_MASK 0x10
#define POWER_SELECT_DCDC_FAULT_OC_MASK 0x08
#define POWER_SELECT_DCDC_FAULT_OT_MASK 0x04

DECLARE_FSM(dcdc);

StatusCode init_dcdc(void);

// AUX BAT DEFINITIONS

#define POWER_SELECT_AUX_BAT_VALID_ADDR \
  { GPIO_PORT_C, 1 }
#define POWER_SELECT_AUX_BAT_VSENSE_ADDR \
  { GPIO_PORT_C, 2 }
#define POWER_SELECT_AUX_BAT_ISENSE_ADDR \
  { GPIO_PORT_C, 3 }
#define POWER_SELECT_AUX_BAT_TSENSE_ADDR \
  { GPIO_PORT_C, 4 }

#define POWER_SELECT_AUX_BAT_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_AUX_BAT_MAX_CURRENT_MA 37500
#define POWER_SELECT_AUX_BAT_MAX_TEMP_C 1

#define POWER_SELECT_AUX_BAT_STATUS_MASK 0x40
#define POWER_SELECT_AUX_BAT_FAULT_OV_MASK 0x80
#define POWER_SELECT_AUX_BAT_FAULT_OC_MASK 0x40
#define POWER_SELECT_AUX_BAT_FAULT_OT_MASK 0x20

DECLARE_FSM(aux_bat);

StatusCode init_aux_bat(void);
