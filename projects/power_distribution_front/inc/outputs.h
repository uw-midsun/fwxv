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
  FRONT_OUTPUT_CENTRE_CONSOLE = 0,
  FRONT_OUTPUT_PEDAL,
  FRONT_OUTPUT_STEERING,
  FRONT_OUTPUT_UV_CUTOFF,
  FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
  FRONT_OUTPUT_CAMERA_DISPLAY,  // aka main display

  FRONT_OUTPUT_LEFT_CAMERA,
  FRONT_OUTPUT_RIGHT_CAMERA,
  FRONT_OUTPUT_MAIN_PI,  // driver display + telemetry pi
  FRONT_OUTPUT_REAR_DISPLAY,
  FRONT_OUTPUT_LEFT_DISPLAY,
  FRONT_OUTPUT_RIGHT_DISPLAY,
  FRONT_OUTPUT_SPEAKER,  // If not using speaker

  FRONT_OUTPUT_5V_SPARE_1,
  FRONT_OUTPUT_5V_SPARE_2,
  FRONT_OUTPUT_SPARE_1,  // Spare_1_Telemetry_PWR
  FRONT_OUTPUT_SPARE_2,  // MCI_PWR
  FRONT_OUTPUT_SPARE_3,
  FRONT_OUTPUT_SPARE_4,  // on rear fan 1's BTS7200 channel
  FRONT_OUTPUT_SPARE_5,  // on rear fan 2's BTS7200 channel

  // Outputs for rear power distribution
  REAR_OUTPUT_BMS,
  REAR_OUTPUT_MCI,
  REAR_OUTPUT_SOLAR_SENSE,
  REAR_OUTPUT_BPS_STROBE_LIGHT,
  REAR_OUTPUT_FAN_1,
  // REAR_OUTPUT_FAN_2,
  // REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
  // REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
  // REAR_OUTPUT_BRAKE_LIGHT,
  // REAR_OUTPUT_REAR_CAMERA,

  // REAR_OUTPUT_5V_SPARE_1,
  // REAR_OUTPUT_5V_SPARE_2,
  // REAR_OUTPUT_SPARE_1,   // on Telemetry
  // REAR_OUTPUT_SPARE_2,   // on Left Display
  // REAR_OUTPUT_SPARE_3,
  // REAR_OUTPUT_SPARE_4,   // on pedal's BTS7200 channel
  // REAR_OUTPUT_SPARE_5,   // on steering's BTS7200 channel
  // REAR_OUTPUT_SPARE_6,   // on right camera's BTS7200 channel
  // REAR_OUTPUT_SPARE_7,   // on main pi's BTS7200 channel
  // REAR_OUTPUT_SPARE_8,   // on driver display's BTS7200 channel
  // REAR_OUTPUT_SPARE_9,   // on centre console's BTS7200 channel
  // REAR_OUTPUT_SPARE_10,  // on rear display's BTS7200 channel
  // REAR_OUTPUT_SPARE_11,  // UV_cutoff_Pwr

  NUM_OUTPUTS,
} Output;

typedef enum {
  OUTPUT_STATE_OFF = 0,
  OUTPUT_STATE_ON,
  NUM_OUTPUT_STATES,
} OutputState;

// List of output groups that can be controlled as individual entity
typedef enum {
  OUTPUT_GROUP_ALL = 0,  // Reserved Value
  // Lights
  OUTPUT_GROUP_LEFT_TURN,
  OUTPUT_GROUP_RIGHT_TURN,
  OUTPUT_GROUP_HAZARD,
  NUM_OUTPUT_GROUPS,
} OutputGroup;

typedef struct OutputMeasurementConfig {
  uint8_t mux_val;
} OutputMeasurementConfig;

typedef struct OutputGroupDef {
  uint8_t num_outputs;
  Output outputs[];
} OutputGroupDef;

extern const BtsLoadSwitchOutput g_output_config[NUM_OUTPUTS];
extern const OutputGroupDef *g_output_group_map[NUM_OUTPUT_GROUPS];

// Initializes all outputs
StatusCode pd_output_init(void);

// Sets an output group to an output state
StatusCode pd_set_output_group(OutputGroup group, OutputState state);
