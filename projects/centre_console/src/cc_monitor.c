#include "cc_monitor.h"

#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "motor_controller_getters.h"
#include "pca9555_gpio_expander.h"
#include "power_distribution_front_getters.h"
#include "steering_digital_task.h"

// Multiplication factor to convert CAN motor velocity (cm/s) into drive output velocity (mm/s)
#define CONVERT_VELOCITY 5

// Multiplication factor to convert CAN drive output velocity to kph
#define CONVERT_VELCOITY_TO_KPH 0.0036

#define TEMP_I2C_ADDRESS 0x55
Pca9555GpioAddress cc_light = { .i2c_address = TEMP_I2C_ADDRESS, .pin = PCA9555_PIN_IO0_0 };

static LocalState state;

void set_local_state() {
  state.cc_toggle = get_drive_output_cruise_control();
  state.drive_state = get_drive_output_drive_state();
  state.target_velocity = get_drive_output_target_velocity();
  state.regen_braking = get_drive_output_regen_braking();
}

StatusCode update_state() {
  if (get_pedal_output_brake_output()) {
    state.cc_toggle = false;
  }

  // check steering message for CC event (toggle/increase/decrease) or regen braking
  SteeringInterfaceDigitalInput steering_event = get_steering_info_digital_input();

  if (steering_event == STEERING_DIGITAL_INPUT_CC_TOGGLE) {
    state.cc_toggle = (state.cc_toggle) ? !state.cc_toggle : state.cc_toggle;

    if (state.cc_toggle) {
      // Store recent speed from MCI as initial cruise control speed
      state.target_velocity =
          (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) * CONVERT_VELOCITY;

      pca9555_gpio_set_state(&cc_light, PCA9555_GPIO_STATE_HIGH);
    } else if (!state.cc_toggle) {
      state.target_velocity = 0;

      pca9555_gpio_set_state(&cc_light, PCA9555_GPIO_STATE_LOW);
    }
  } else if (steering_event == STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED) {
    state.target_velocity =
        ((state.target_velocity * CONVERT_VELCOITY_TO_KPH) + 1) / CONVERT_VELCOITY_TO_KPH;
  } else if (steering_event == STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED) {
    state.target_velocity =
        ((state.target_velocity * CONVERT_VELCOITY_TO_KPH) - 1) / CONVERT_VELCOITY_TO_KPH;
  } else if (steering_event == STEERING_REGEN_BRAKE_EVENT) {
    state.regen_braking = true;
  }

  return STATUS_CODE_OK;
}

void send_message_to_mci() {
  set_drive_output_drive_state(state.drive_state);
  set_drive_output_cruise_control(state.cc_toggle);
  set_drive_output_target_velocity(state.target_velocity);
  set_drive_output_regen_braking(state.regen_braking);
}

StatusCode monitor_cruise_control() {
  set_local_state();

  if (state.drive_state != DRIVE) {
    return;
  } else {
    update_state();
  }
  send_message_to_mci();

  return STATUS_CODE_OK;
}
