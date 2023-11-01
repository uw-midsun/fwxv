#include "cc_monitor.h"

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

static LocalState state;

void set_local_state() {
  state.cc_enabled = g_tx_struct.drive_output_cruise_control;
  state.drive_state = g_tx_struct.drive_output_drive_state;
  state.target_velocity = g_tx_struct.drive_output_target_velocity;
  state.regen_braking = g_tx_struct.drive_output_regen_braking;
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
  uint8_t cc_info = get_steering_info_digital_input();

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
