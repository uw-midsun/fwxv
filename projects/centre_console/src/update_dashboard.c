#include "update_dashboard.h"

#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "pca9555_gpio_expander.h"

// Multiplication factor to convert CAN motor velocity (cm/s) into drive output velocity (mm/s)
#define CONVERT_VELOCITY 5

// Multiplication factor to convert CAN drive output velocity to kph
#define CONVERT_VELOCITY_TO_KPH 0.0036

#define TEMP_I2C_ADDRESS 0x55
Pca9555GpioAddress cc_light = { .i2c_address = TEMP_I2C_ADDRESS, .pin = PCA9555_PIN_IO0_0 };
Pca9555GpioAddress cc_regen_brake_light = { .i2c_address = TEMP_I2C_ADDRESS,
                                            .pin = PCA9555_PIN_IO0_1 };

Event TEMP_REGEN_BRAKE_EVENT_PRESSED = 0;

// Centre Console State Variables
static uint8_t s_drive_state;
static bool s_cc_enabled;
static bool s_regen_braking;
static uint32_t s_target_velocity;

typedef enum DriveLeds {
  HAZARD_LED = 0,
  LEFT_LED,
  RIGHT_LED,
  CRUISE_LED,
  REGEN_LED,
  POWER_LED,
  NUM_DRIVE_LED,
} DriveLeds;

static Pca9555GpioAddress s_drive_btn_leds[NUM_DRIVE_LED] = {
  [HAZARD_LED] = HAZARD_LED_ADDR,
  [LEFT_LED] = LEFT_LED_ADDR,
  [RIGHT_LED] = RIGHT_LED_ADDR,
  [CRUISE_LED] = CRUISE_LED_ADDR,
  [REGEN_LED] = REGEN_LED_ADDR
};


void update_indicators(uint32_t notif) {
  uint32_t notif = 0;
  notify_get(&notif);

  // Update power btn light
  Pca9555GpioState state = PCA9555_GPIO_STATE_LOW;
  if (get_power_info_power_state() == EE_POWER_ON_STATE || get_power_info_power_state() == EE_POWER_DRIVE_STATE) {
    pca9555_gpio_set_state(&(), PCA9555_GPIO_STATE_LOW); 
  }



}

void update_msg_outputs(uint32_t notif) {
  // Check power press, default to no press
  // If we are not in neutral, ignore
  if (get_drive_state() == NEUTRAL) {
    set_cc_power_control_power_event(EE_CC_PWR_CTL_EVENT_NONE);
    if (notify_check_event(&notif, POWER_BUTTON_EVENT)) {
      if (get_pedal_output_brake_output() > 0) {
        set_cc_power_control_power_event(EE_CC_PWR_CTL_EVENT_BTN_AND_BRAKE);
      } else {
        set_cc_power_control_power_event(EE_CC_PWR_CTL_EVENT_BTN);
      }
    }
  }

  if ()
}

void update_state() {
  if (get_pedal_output_brake_output()) {
    state.cc_enabled = false;
    return;
  }

  // Check for regen brake event
  uint32_t master_task_notif;
  notify_get(&master_task_notif);
  if (notify_check_event(&master_task_notif, TEMP_REGEN_BRAKE_EVENT_PRESSED)) {
    state.regen_braking = !state.regen_braking;
    if (state.regen_braking) {
      pca9555_gpio_set_state(&cc_regen_brake_light, PCA9555_GPIO_STATE_LOW);
    }
  }

  // Check steering message for cc event (toggle/increase/decrease)
  uint8_t cc_info = get_steering_info_input_cc();

  if (cc_info & STEERING_CC_TOGGLE_MASK) {
    if (!state.cc_enabled) {
      // Store recent speed from MCI as initial cruise control speed
      unsigned int convert_velocity = CONVERT_VELOCITY;
      float converted_val =
          (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) * CONVERT_VELOCITY;
      state.target_velocity = (unsigned int)converted_val;

      state.cc_enabled = true;
      pca9555_gpio_set_state(&cc_light, PCA9555_GPIO_STATE_HIGH);
    } else {
      state.target_velocity = 0;

      state.cc_enabled = false;
      pca9555_gpio_set_state(&cc_light, PCA9555_GPIO_STATE_LOW);
    }
  } else if (cc_info & STEERING_CC_INCREASE_SPEED_MASK) {
    state.target_velocity =
        ((state.target_velocity * CONVERT_VELOCITY_TO_KPH) + 1) / CONVERT_VELOCITY_TO_KPH;
  } else if (cc_info & STEERING_CC_DECREASE_SPEED_MASK) {
    state.target_velocity =
        ((state.target_velocity * CONVERT_VELOCITY_TO_KPH) - 1) / CONVERT_VELOCITY_TO_KPH;
  }
}

void update_drive_output() {
  set_drive_output_drive_state(state.drive_state);
  set_drive_output_cruise_control(state.cc_enabled);
  set_drive_output_target_velocity(state.target_velocity);
  set_drive_output_regen_braking(state.regen_braking);
}

void monitor_cruise_control() {
  set_local_state();

  if (state.drive_state != DRIVE) {
    return;
  } else {
    update_state();
  }
  update_drive_output();
}
