#include "outputs.h"

static Bts7xxxPin s_front_pin_centre_console_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_CENTRE_CONSOLE_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_steering_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_STEERING_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_pedal_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_PEDAL_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_daytime_running_lights_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_DAYTIME_RUNNING_LIGHTS_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_front_left_turn_light_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_front_right_turn_light_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_main_pi_camera_display_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_MAIN_PI_CAMERA_DISPLAY_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_centre_console_rear_display_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_CENTRE_CONSOLE_REAR_DISPLAY_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_pedal_steering_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_PEDAL_STEERING_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_front_pin_front_left_right_turn_light_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_FRONT_LEFT_RIGHT_TURN_LIGHT_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_mci_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_MCI_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_bms_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_BMS_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_solar_sense_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_SOLAR_SENSE_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_fan_1_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_FAN_1_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_strobe_light_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_STROBE_LIGHT_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_brake_light_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_BRAKE_LIGHT_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_strobe_light_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_STROBE_LIGHT_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_rear_left_right_turn_light_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_REAR_LEFT_RIGHT_TURN_LIGHT_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_rear_pin_fan_1_2_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_FAN_1_2_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_left_rear_cam_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)LEFT_REAR_CAMERA_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_right_cam_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)RIGHT_CAMERA_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};
static Bts7xxxPin s_left_rear_right_cam_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_RIGHT_CAMERA_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

// TODO: Update select states
BtsLoadSwitchOutput g_output_config[NUM_OUTPUTS] = {
  [FRONT_OUTPUT_CENTRE_CONSOLE] = {
    .enable_pin = &s_front_pin_centre_console_en,
    .select_pin = &s_front_pin_centre_console_rear_display_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_CENTRE_CONSOLE_REAR_DISPLAY,
  },
  [FRONT_OUTPUT_PEDAL] = {
    .enable_pin = &s_front_pin_pedal_en,
    .select_pin = &s_front_pin_pedal_steering_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_LOW  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_PEDAL_STEERING,
  },
  [FRONT_OUTPUT_STEERING] = {
    .enable_pin = &s_front_pin_steering_en,
    .select_pin = &s_front_pin_pedal_steering_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_PEDAL_STEERING,
  },
  [FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT] = {
    .enable_pin = &s_front_pin_front_left_turn_light_en,
    .select_pin = &s_rear_pin_rear_left_right_turn_light_dsel,
    .select_state =  { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_FRONT_REAR_LEFT_RIGHT_TURN_LIGHT,
  },
  [FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT] = {
    .enable_pin = &s_front_pin_front_right_turn_light_en,
    .select_pin = &s_rear_pin_rear_left_right_turn_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_FRONT_REAR_LEFT_RIGHT_TURN_LIGHT,
  },
  [FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS] = {
    .enable_pin = &s_front_pin_daytime_running_lights_en,
    .select_pin = NULL,
    .select_state = {0},
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_DAYTIME_RUNNING_LIGHTS_BRAKE_LIGHTS,
  },
  [FRONT_OUTPUT_CAMERA_DISPLAY] = {
    .enable_pin = &s_front_pin_main_pi_camera_display_en,
    .select_pin = NULL,
    .select_state = {0},
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_INFOTAINMENT_DISPLAY_BMS,
  },
  [REAR_OUTPUT_BMS] = {
    .enable_pin = &s_rear_pin_bms_en,
    .select_pin = NULL,
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_INFOTAINMENT_DISPLAY_BMS,
  },
  [REAR_OUTPUT_MCI] = {
    .enable_pin = &s_rear_pin_mci_en,
    .select_pin = NULL,
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_MCI,
  },
  [REAR_OUTPUT_SOLAR_SENSE] = {
    .enable_pin = &s_rear_pin_solar_sense_en,
    .select_state = {0},
    .select_pin = NULL,
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_SPEAKER_SOLAR,
  },
  [REAR_OUTPUT_BPS_STROBE_LIGHT] = {
    .enable_pin = &s_rear_pin_strobe_light_en,
    .select_pin = &s_rear_pin_strobe_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_LEFT_RIGHT_DISPLAY_STROBE_LIGHTS,
  },
  [REAR_OUTPUT_FAN_1] = {
    .enable_pin = &s_rear_pin_fan_1_en,
    .select_pin = &s_rear_pin_fan_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_FAN_1_2,
  },
  [LEFT_REAR_CAMERA] = {
    .enable_pin = &s_left_rear_cam_en,
    .select_pin = &s_left_rear_right_cam_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_LOW  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_LEFT_RIGHT_REAR_CAMERA,
  },
  [RIGHT_CAMERA] = {
    .enable_pin = &s_right_cam_en,
    .select_pin = &s_left_rear_right_cam_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
    .mux_val = MUX_SEL_LEFT_RIGHT_REAR_CAMERA,
  },
};

// Output Group Definitions
static OutputGroupDef s_output_group_left_signal = {
  .num_outputs = 1,
  .outputs = { FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT },
};

static OutputGroupDef s_output_group_right_signal = {
  .num_outputs = 1,
  .outputs = { FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT },
};

static OutputGroupDef s_output_group_hazards = {
  .num_outputs = 2,
  .outputs = { FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT, FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT },
};

static OutputGroupDef s_output_group_test = {
  .num_outputs = 2,
  .outputs = { FRONT_OUTPUT_STEERING, FRONT_OUTPUT_PEDAL }
};

const OutputGroupDef *g_output_group_map[NUM_OUTPUT_GROUPS] = {
  [OUTPUT_GROUP_ALL] = NULL,  // Special case
  [OUTPUT_GROUP_LEFT_TURN] = &s_output_group_left_signal,
  [OUTPUT_GROUP_RIGHT_TURN] = &s_output_group_right_signal,
  [OUTPUT_GROUP_HAZARD] = &s_output_group_hazards,
  [OUTPUT_GROUP_TEST] = &s_output_group_test,
};
