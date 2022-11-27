#pragma once

#include "adc.h"
// #include "can_codegen.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "power_select_power_supply_task.h"
#include "power_select_setters.h"

#define POWER_SELECT_DCDC_VALID_ADDR \
  { GPIO_PORT_A, 10 }
#define POWER_SELECT_DCDC_VSENSE_ADDR \
  { GPIO_PORT_A, 2 }
#define POWER_SELECT_DCDC_ISENSE_ADDR \
  { GPIO_PORT_A, 7 }
#define POWER_SELECT_DCDC_TSENSE_ADDR \
  { GPIO_PORT_A, 7 }

#define POWER_SELECT_DCDC_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_DCDC_MAX_CURRENT_MA 37500
#define POWER_SELECT_DCDC_MAX_TEMP_C 1

#define POWER_SELECT_DCDC_STATUS_MASK 0x02
#define POWER_SELECT_DCDC_FAULT_OV_MASK 0x10
#define POWER_SELECT_DCDC_FAULT_OC_MASK 0x08
#define POWER_SELECT_DCDC_FAULT_OT_MASK 0x04
