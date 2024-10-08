#pragma once

// General-purpose module for manipulating the outputs that power distribution controls.
// Requires GPIO, interrupts, soft timers, ADC, and I2C to be initialized.
//
// An output is an abstraction of "something that PD can turn on and off".
// This module provides a uniform interface for manipulating outputs implemented through GPIO or
// an IO expander, and through a BTS7200 or BTS7040 load switch or not.

#include <stdbool.h>
#include <stdint.h>

#include "bts_load_switch.h"
#include "gpio.h"
#include "i2c.h"
#include "pca9555_gpio_expander.h"
#include "pin_defs.h"

// Please don't change the numerical values of the outputs so downstream tools can rely on them.
// Add any new outputs at the end, and if a spare gets a proper name, just rename the spare output.
typedef enum {
  // Outputs for front power distribution
  SOLAR_SENSE_1 = 0,
  SOLAR_SENSE_2,
  PEDAL,
  DRL_LEFT,
  DRL_RIGHT,
  CENTER_CONSOLE,
  BMS_DCDC,
  MCI,
  SPARE_12V_1,
  SPARE_12V_2,
  BMS_AUX,
  DRIVER_FAN,
  REAR_CAM_LCD,
  SPARE_5V_DCDC,
  TELEMETRY,
  SPARE_5V_AUX,
  NUM_POWER_FSM_OUTPUTS,
} OutputPowerFsm;

typedef enum {
  BPS_LIGHT = NUM_POWER_FSM_OUTPUTS,
  RIGHT_TURN,
  LEFT_TURN,
  BRAKE_LIGHT,
  NUM_OUTPUTS,
} OutputLights;

typedef enum {
  OUTPUT_STATE_OFF = 0,
  OUTPUT_STATE_ON,
  NUM_OUTPUT_STATES,
} OutputState;

// List of output groups that can be controlled as individual entity
typedef enum {
  OUTPUT_GROUP_ALL = 0,  // Reserved Value
  // Lights
  OUTPUT_GROUP_LIGHTS_LEFT_TURN,
  OUTPUT_GROUP_LIGHTS_RIGHT_TURN,
  OUTPUT_GROUP_DRL_LEFT,
  OUTPUT_GROUP_DRL_RIGHT,
  OUTPUT_GROUP_LIGHTS_HAZARD,
  OUTPUT_GROUP_BRAKE,
  // Power States
  OUTPUT_GROUP_POWER_OFF,
  OUTPUT_GROUP_POWER_DRIVE,
  OUTPUT_GROUP_POWER_FAULT,
  OUTPUT_GROUP_BPS,
  OUTPUT_GROUP_TEST,
  NUM_OUTPUT_GROUPS,
} OutputGroup;

typedef enum {
  ISENSE_PEDAL_STEERING = 6,
  ISENSE_SOLAR,
  ISENSE_LEFT_RIGHT_TURN,
  ISENSE_DRL_BRAKE,
  ISENSE_CENTER_CONSOLE_BMS_DCDC,
  ISENSE_BMS_AUX,
  ISENSE_TELEMETRY,
  ISENSE_CAMERA,
  ISENSE_DCDC_SPARE,
  ISENSE_MCI_BPS_LIGHT,
} ISenseMux;

#define ISENSE_GAIN 2.7
#define ISENSE_BIAS 1
#define ISENSE_R1_KOHMS 1
#define ISENSE_R2_KOHMS 1.21
#define ISENSE_VOLTAGE_DIVIDER (ISENSE_R2_KOHMS / (ISENSE_R1_KOHMS + ISENSE_R2_KOHMS))

typedef struct OutputMeasurementConfig {
  uint8_t mux_val;
} OutputMeasurementConfig;

typedef struct OutputGroupDef {
  uint8_t num_outputs;
  uint8_t outputs[];
} OutputGroupDef;

extern BtsLoadSwitchOutput g_output_config[NUM_OUTPUTS];
extern const OutputGroupDef *g_output_group_map[NUM_OUTPUT_GROUPS];

// Initializes all outputs
StatusCode pd_output_init(void);

// Sets an output group to an output state
StatusCode pd_set_output_group(OutputGroup group, OutputState state);

StatusCode pd_set_active_output_group(OutputGroup group);
