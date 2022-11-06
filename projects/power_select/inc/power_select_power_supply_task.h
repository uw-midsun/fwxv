#pragma once

#include "adc.h"
#include "can_codegen.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"

#define NUM_POWER_SUPPLY_STATES 2
#define NUM_POWER_SUPPLY_TRANSITIONS 2

#define POWER_SELECT_PWR_SUP_VALID_ADDR \
  { GPIO_PORT_A, 10 }
#define POWER_SELECT_PWR_SUP_VSENSE_ADDR \
  { GPIO_PORT_A, 2 }
#define POWER_SELECT_PWR_SUP_ISENSE_ADDR \
  { GPIO_PORT_A, 7 }

#define POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_PWR_SUP_MAX_CURRENT_MA 37500

#define POWER_SELECT_PWR_SUP_STATUS_MASK 0x01
#define POWER_SELECT_PWR_SUP_FAULT_OC_MASK 0x01
#define POWER_SELECT_PWR_SUP_FAULT_OV_MASK 0x02

DECLARE_FSM(power_supply);

extern const GpioAddress g_power_select_valid_pin;
extern const GpioAddress g_power_select_voltage_pin;
extern const GpioAddress g_power_select_current_pin;

typedef enum PowerSupplyStateId {
  POWER_SUPPLY_INACTIVE = 0,
  POWER_SUPPLY_ACTIVE,
} PowerSupplyStateId;

StatusCode init_power_supply(void);
