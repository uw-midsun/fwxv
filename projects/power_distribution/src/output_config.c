#include "outputs.h"

static Bts7xxxPin s_solar_sense_1_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)SOLAR_SENSE_1_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_solar_sense_2_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)SOLAR_SENSE_2_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_solar_sense_1_2_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)SOLAR_SENSE_1_2_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_pedal_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)PEDAL_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_steering_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)STEERING_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_pedal_steering_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)PEDAL_STEERING_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_turn_left_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)LEFT_TURN_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_right_turn_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)RIGHT_TURN_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_left_right_turn_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)LEFT_RIGHT_TURN_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_drl_light_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)DRL_LIGHT_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_brake_light_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)BRAKE_LIGHT_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_drl_brake_light_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)DRL_BRAKE_LIGHT_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_center_console_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)CENTER_CONSOLE_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_bms_dcdc_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)BMS_DCDC_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_center_console_bms_dcdc_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)CENTER_CONSOLE_BMS_DCDC_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_mci_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)MCI_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_bps_light_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)BPS_LIGHT_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_mci_bps_light_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)MCI_BPS_LIGHT_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_spare_12v_1_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)SPARE_12V_1_EN,  // NOLINT
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

static Bts7xxxPin s_spare_12v_2_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)SPARE_12V_2_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_spare_12v_1_2_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)SPARE_12V_1_2_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_bms_aux_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)BMS_AUX_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_driver_fan_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)DRIVER_FAN_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_bms_aux_driver_fan_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)BMS_AUX_DRIVER_FAN_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_rear_cam_lcd_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_CAM_LCD_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_spare_5v_dcdc_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)SPARE_5V_DCDC_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_rear_cam_lcd_spare_5v_dcdc_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_CAM_LCD_SPARE_5V_DCDC_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_telemetry_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)TELEMETRY_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_spare_5v_aux_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)SPARE_5V_AUX_EN,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

static Bts7xxxPin s_telemetry_spare_5v_aux_dsel = {
  .pin_pca9555 = &(Pca9555GpioAddress)TELEMETRY_SPARE_5V_AUX_DSEL,  // NOLINT
  .pin_type = BTS7XXX_PIN_PCA9555,
};

// TODO: Update select states
BtsLoadSwitchOutput g_output_config[NUM_OUTPUTS] = {
  [SOLAR_SENSE_1] = {
    .enable_pin = &s_solar_sense_1_en,
    .select_pin = &s_solar_sense_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [SOLAR_SENSE_2] = {
    .enable_pin = &s_solar_sense_2_en,
    .select_pin = &s_solar_sense_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [PEDAL] = {
    .enable_pin = &s_pedal_en,
    .select_pin = &s_pedal_steering_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [STEERING] = {
    .enable_pin = &s_steering_en,
    .select_pin = &s_pedal_steering_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [LEFT_TURN] = {
    .enable_pin = &s_turn_left_en,
    .select_pin = &s_left_right_turn_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [RIGHT_TURN] = {
    .enable_pin = &s_right_turn_en,
    .select_pin = &s_left_right_turn_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [DRL_LIGHT] = {
    .enable_pin = &s_drl_light_en,
    .select_pin = &s_drl_brake_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [BRAKE_LIGHT] = {
    .enable_pin = &s_brake_light_en,
    .select_pin = &s_drl_brake_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [CENTER_CONSOLE] = {
    .enable_pin = &s_center_console_en,
    .select_pin = &s_center_console_bms_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [BMS_DCDC] = {
    .enable_pin = &s_bms_dcdc_en,
    .select_pin = &s_center_console_bms_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [MCI] = {
    .enable_pin = &s_mci_en,
    .select_pin = &s_mci_bps_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [BPS_LIGHT] = {
    .enable_pin = &s_bps_light_en,
    .select_pin = &s_mci_bps_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [SPARE_12V_1] = {
    .enable_pin = &s_spare_12v_1_en,
    .select_pin = &s_spare_12v_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [SPARE_12V_2] = {
    .enable_pin = &s_spare_12v_2_en,
    .select_pin = &s_spare_12v_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [BMS_AUX] = {
    .enable_pin = &s_bms_aux_en,
    .select_pin = &s_bms_aux_driver_fan_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [DRIVER_FAN] = {
    .enable_pin = &s_driver_fan_en,
    .select_pin = &s_bms_aux_driver_fan_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [REAR_CAM_LCD] = {
    .enable_pin = &s_rear_cam_lcd_en,
    .select_pin = &s_rear_cam_lcd_spare_5v_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [SPARE_5V_DCDC] = {
    .enable_pin = &s_spare_5v_dcdc_en,
    .select_pin = &s_rear_cam_lcd_spare_5v_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [TELEMETRY] = {
    .enable_pin = &s_telemetry_en,
    .select_pin = &s_telemetry_spare_5v_aux_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
  },
  [SPARE_5V_AUX] = {
    .enable_pin = &s_spare_5v_aux_en,
    .select_pin = &s_telemetry_spare_5v_aux_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .resistor = 0,
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
  .outputs = { LEFT_TURN },
};

static OutputGroupDef s_output_group_right_signal = {
  .num_outputs = 1,
  .outputs = { RIGHT_TURN },
};

static OutputGroupDef s_output_group_hazards = {
  .num_outputs = 2,
  .outputs = { LEFT_TURN, RIGHT_TURN },
};

static OutputGroupDef s_output_group_test = {
  .num_outputs = 2, .outputs = { FRONT_OUTPUT_STEERING, FRONT_OUTPUT_PEDAL }
};

const OutputGroupDef *g_output_group_map[NUM_OUTPUT_GROUPS] = {
  [OUTPUT_GROUP_ALL] = NULL,  // Special case
  [OUTPUT_GROUP_LEFT_TURN] = &s_output_group_left_signal,
  [OUTPUT_GROUP_RIGHT_TURN] = &s_output_group_right_signal,
  [OUTPUT_GROUP_HAZARD] = &s_output_group_hazards,
  [OUTPUT_GROUP_TEST] = &s_output_group_test,
};
