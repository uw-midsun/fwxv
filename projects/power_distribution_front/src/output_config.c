#include "output_config.h"

static Bts7xxxPin s_front_pin_centre_console_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_CENTRE_CONSOLE_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_steering_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_STEERING_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_pedal_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_PEDAL_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_daytime_running_lights_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_DAYTIME_RUNNING_LIGHTS_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_front_left_turn_light_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_front_right_turn_light_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_main_pi_camera_display_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_MAIN_PI_CAMERA_DISPLAY_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_centre_console_rear_display_dsel = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_CENTRE_CONSOLE_REAR_DISPLAY_DSEL,
  .pin_type = BTS7XXX_PIN_PCA9555,
},
static Bts7xxxPin s_front_pin_pedal_steering_dsel = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_PEDAL_STEERING_DSEL,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_front_pin_front_left_right_turn_light_dsel = { 
  .pin_pca9555 = &(Pca9555GpioAddress)FRONT_PIN_FRONT_LEFT_RIGHT_TURN_LIGHT_DSEL,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_mci_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_MCI_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_bms_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_BMS_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_solar_sense_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_SOLAR_SENSE_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_fan_1_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_FAN_1_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_strobe_light_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_STROBE_LIGHT_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_brake_light_en = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_BRAKE_LIGHT_EN,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_strobe_light_dsel = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_STROBE_LIGHT_DSEL,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_rear_left_right_turn_light_dsel = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_REAR_LEFT_RIGHT_TURN_LIGHT_DSEL,
  .pin_type = BTS7XXX_PIN_PCA9555,
}, 
static Bts7xxxPin s_rear_pin_fan_1_2_dsel = { 
  .pin_pca9555 = &(Pca9555GpioAddress)REAR_PIN_FAN_1_2_DSEL,
  .pin_type = BTS7XXX_PIN_PCA9555,
},

// TODO: Update select states
BtsLoadSwitchOutput g_output_config[NUM_OUTPUTS] = {
  [FRONT_OUTPUT_CENTRE_CONSOLE] = {
    .enable_pin = &s_front_pin_centre_console_en,
    .select_pin = &s_front_pin_centre_console_rear_display_dsel,
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [FRONT_OUTPUT_PEDAL] = {
    .enable_pin = &s_front_pin_pedal_en,
    .select_pin = &s_front_pin_pedal_steering_dsel, 
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [FRONT_OUTPUT_STEERING] = {
    .enable_pin = &s_front_pin_steering_en,
    .select_pin = &s_front_pin_pedal_steering_dsel, 
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT] = {
    .enable_pin = &s_front_pin_front_left_turn_light_en,
    .select_pin = &s_rear_pin_rear_left_right_turn_light_dsel, 
    .select_state =  { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT] = {
    .enable_pin = &s_front_pin_front_right_turn_light_en,
    .select_pin = &s_rear_pin_rear_left_right_turn_light_dsel, 
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS] = {
    .enable_pin = &s_front_pin_daytime_running_lights_en,
    .select_pin = NULL, 
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  // TODO: Brake light will be put on separate line in next rev
  //[REAR_OUTPUT_BRAKE_LIGHT] = {
  //  .enable_pin = &s_front_pin_daytime_running_lights_en,
  //  .select_pin = , 
  //  .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  //},
  [FRONT_OUTPUT_CAMERA_DISPLAY] = {
    .enable_pin = &s_front_pin_main_pi_camera_display_en,
    .select_pin = NULL, 
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },

  [REAR_OUTPUT_BMS] = {
    .enable_pin = &s_rear_pin_bms_en,
    .select_pin = NULL, 
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [REAR_OUTPUT_MCI] = {
    .enable_pin = s_rear_pin_mci_en,
    .select_pin = NULL, 
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [REAR_OUTPUT_SOLAR_SENSE] = {
    .enable_pin = &s_rear_pin_solar_sense_en,
    .select_pin = NULL, 
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [REAR_OUTPUT_BPS_STROBE_LIGHT] = {
    .enable_pin = s_rear_pin_strobe_light_en,
    .select_pin = &s_rear_pin_strobe_light_dsel, 
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },
  [REAR_OUTPUT_FAN_1] = {
    .enable_pin = &s_rear_pin_fan_1_en,
    .select_pin = s_rear_pin_fan_1_2_dsel, 
    .select_state = { .select_state_pca9555 = PCA9555_GPIO_STATE_HIGH  },
    .sense_pin = &(GpioAddress)PD_MUX_OUTPUT_PIN,
  },  
}
