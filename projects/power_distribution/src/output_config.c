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

static Bts7xxxPin s_drl_left_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)DRL_EN_LEFT,  // NOLINT
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

static Bts7xxxPin s_drl_right_en = {
  .pin_pca9555 = &(Pca9555GpioAddress)DRL_EN_RIGHT,  // NOLINT
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
    .mux_val = ISENSE_SOLAR,
  },
  [SOLAR_SENSE_2] = {
    .enable_pin = &s_solar_sense_2_en,
    .select_pin = &s_solar_sense_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_SOLAR,
  },
  [PEDAL] = {
    .enable_pin = &s_pedal_en,
    .select_pin = &s_pedal_steering_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_PEDAL_STEERING,
  },
  [DRL_LEFT] = {
    .enable_pin = &s_drl_left_en,
    .select_pin = &s_pedal_steering_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_PEDAL_STEERING,
  },
  [LEFT_TURN] = {
    .enable_pin = &s_turn_left_en,
    .select_pin = &s_left_right_turn_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_LEFT_RIGHT_TURN,
  },
  [RIGHT_TURN] = {
    .enable_pin = &s_right_turn_en,
    .select_pin = &s_left_right_turn_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_LEFT_RIGHT_TURN,
  },
  [DRL_RIGHT] = {
    .enable_pin = &s_drl_right_en,
    .select_pin = &s_drl_brake_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_DRL_BRAKE,
  },
  [BRAKE_LIGHT] = {
    .enable_pin = &s_brake_light_en,
    .select_pin = &s_drl_brake_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_DRL_BRAKE,
  },
  [CENTER_CONSOLE] = {
    .enable_pin = &s_center_console_en,
    .select_pin = &s_center_console_bms_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_CENTER_CONSOLE_BMS_DCDC,
  },
  [BMS_DCDC] = {
    .enable_pin = &s_bms_dcdc_en,
    .select_pin = &s_center_console_bms_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_CENTER_CONSOLE_BMS_DCDC,
  },
  [MCI] = {
    .enable_pin = &s_mci_en,
    .select_pin = &s_mci_bps_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_MCI_BPS_LIGHT,
  },
  [BPS_LIGHT] = {
    .enable_pin = &s_bps_light_en,
    .select_pin = &s_mci_bps_light_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_MCI_BPS_LIGHT,
  },
  [SPARE_12V_1] = {
    .enable_pin = &s_spare_12v_1_en,
    .select_pin = &s_spare_12v_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_DCDC_SPARE,
  },
  [SPARE_12V_2] = {
    .enable_pin = &s_spare_12v_2_en,
    .select_pin = &s_spare_12v_1_2_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_DCDC_SPARE,
  },
  [BMS_AUX] = {
    .enable_pin = &s_bms_aux_en,
    .select_pin = &s_bms_aux_driver_fan_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_BMS_AUX,
  },
  [DRIVER_FAN] = {
    .enable_pin = &s_driver_fan_en,
    .select_pin = &s_bms_aux_driver_fan_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_BMS_AUX,
  },
  [REAR_CAM_LCD] = {
    .enable_pin = &s_rear_cam_lcd_en,
    .select_pin = &s_rear_cam_lcd_spare_5v_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_CAMERA,
  },
  [SPARE_5V_DCDC] = {
    .enable_pin = &s_spare_5v_dcdc_en,
    .select_pin = &s_rear_cam_lcd_spare_5v_dcdc_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_CAMERA,
  },
  [TELEMETRY] = {
    .enable_pin = &s_telemetry_en,
    .select_pin = &s_telemetry_spare_5v_aux_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_TELEMETRY,
  },
  [SPARE_5V_AUX] = {
    .enable_pin = &s_spare_5v_aux_en,
    .select_pin = &s_telemetry_spare_5v_aux_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN, // NOLINT
    .mux_val = ISENSE_TELEMETRY,
  },
};

// Output Group Definitions
static OutputGroupDef s_output_group_left_signal = {
  .num_outputs = 1,
  .outputs = { LEFT_TURN },
};

static OutputGroupDef s_output_group_right_drl = {
  .num_outputs = 1,
  .outputs = { DRL_RIGHT },
};

static OutputGroupDef s_output_group_left_drl = {
  .num_outputs = 1,
  .outputs = { DRL_LEFT },
};

static OutputGroupDef s_output_group_right_signal = {
  .num_outputs = 1,
  .outputs = { RIGHT_TURN },
};

static OutputGroupDef s_output_group_hazards = {
  .num_outputs = 2,
  .outputs = { LEFT_TURN, RIGHT_TURN },
};

static OutputGroupDef s_output_group_brake = { .num_outputs = 1, .outputs = { BRAKE_LIGHT } };

static OutputGroupDef s_output_group_bps = { .num_outputs = 1, .outputs = { BPS_LIGHT } };

static OutputGroupDef s_output_group_power_off = { .num_outputs = 8,
                                                   .outputs = { CENTER_CONSOLE, BMS_DCDC, BMS_AUX,
                                                                PEDAL, DRL_RIGHT, DRL_LEFT,
                                                                REAR_CAM_LCD, SPARE_5V_AUX } };

static OutputGroupDef s_output_group_power_drive = {
  .num_outputs = 10,
  .outputs = { CENTER_CONSOLE, BMS_DCDC, BMS_AUX, PEDAL, MCI, DRL_RIGHT, DRL_LEFT, DRIVER_FAN,
               REAR_CAM_LCD, SPARE_5V_AUX }
};

static OutputGroupDef s_output_group_power_fault = { .num_outputs = 8,
                                                     .outputs = { CENTER_CONSOLE, BMS_DCDC, PEDAL,
                                                                  DRL_LEFT, DRL_RIGHT, BPS_LIGHT,
                                                                  REAR_CAM_LCD, SPARE_5V_AUX } };

const OutputGroupDef *g_output_group_map[NUM_OUTPUT_GROUPS] = {
  [OUTPUT_GROUP_ALL] = NULL,  // Special case
  [OUTPUT_GROUP_LIGHTS_LEFT_TURN] = &s_output_group_left_signal,
  [OUTPUT_GROUP_LIGHTS_RIGHT_TURN] = &s_output_group_right_signal,
  [OUTPUT_GROUP_DRL_LEFT] = &s_output_group_left_drl,
  [OUTPUT_GROUP_DRL_RIGHT] = &s_output_group_right_drl,
  [OUTPUT_GROUP_LIGHTS_HAZARD] = &s_output_group_hazards,
  [OUTPUT_GROUP_BRAKE] = &s_output_group_brake,
  [OUTPUT_GROUP_POWER_OFF] = &s_output_group_power_off,
  [OUTPUT_GROUP_POWER_DRIVE] = &s_output_group_power_drive,
  [OUTPUT_GROUP_POWER_FAULT] = &s_output_group_power_fault,
  [OUTPUT_GROUP_BPS] = &s_output_group_bps,
};
