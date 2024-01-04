#include "update_dashboard.h"

#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "pca9555_gpio_expander.h"
#include "seg_display.h"

// Multiplication factor to convert CAN drive output velocity (cm/s) to kph
#define CONVERT_VELOCITY_TO_KPH 0.036

SegDisplay all_displays = ALL_DISPLAYS;

// Centre Console State Variables
static bool s_cc_enabled;
static bool s_regen_braking;
static bool s_hazard_state;
static uint32_t s_target_velocity;
static uint32_t s_last_power_state = EE_POWER_OFF_STATE;
static uint8_t s_last_lights_state = EE_STEERING_LIGHTS_OFF_STATE;

typedef enum DriveLeds {
  HAZARD_LED = 0,
  REGEN_LED,
  POWER_LED,
  LEFT_LED,
  RIGHT_LED,
  CRUISE_LED,
  LIGHTS_LED,
  NUM_DRIVE_LED,
} DriveLeds;

static Pca9555GpioAddress s_output_leds[NUM_DRIVE_LED] = {
  [POWER_LED] = POWER_LED_ADDR,   [HAZARD_LED] = HAZARD_LED_ADDR, [LEFT_LED] = LEFT_LED_ADDR,
  [RIGHT_LED] = RIGHT_LED_ADDR,   [CRUISE_LED] = CRUISE_LED_ADDR, [REGEN_LED] = REGEN_LED_ADDR,
  [LIGHTS_LED] = LIGHTS_LED_ADDR,
};

void update_indicators(uint32_t notif) {
  // Update hazard light
  if (notify_check_event(&notif, HAZARD_BUTTON_EVENT)) {
    if (!s_hazard_state) {
      s_hazard_state = true;
      pca9555_gpio_set_state(&s_output_leds[HAZARD_LED], PCA9555_GPIO_STATE_HIGH);
    }
  } else {
    if (s_hazard_state) {
      s_hazard_state = false;
      pca9555_gpio_set_state(&s_output_leds[HAZARD_LED], PCA9555_GPIO_STATE_LOW);
    }
  }
  // Update regen light
  if (notify_check_event(&notif, REGEN_BUTTON_EVENT)) {
    if (s_regen_braking) {
      s_regen_braking = false;
      pca9555_gpio_set_state(&s_output_leds[REGEN_LED], PCA9555_GPIO_STATE_LOW);
    } else {
      s_regen_braking = true;
      pca9555_gpio_set_state(&s_output_leds[REGEN_LED], PCA9555_GPIO_STATE_HIGH);
    }
  }

  // Update power btn light if power_state has changed
  if (get_power_info_power_state() != s_last_power_state) {
    if (get_power_info_power_state() == EE_POWER_ON_STATE ||
        get_power_info_power_state() == EE_POWER_DRIVE_STATE) {
      pca9555_gpio_set_state(&s_output_leds[POWER_LED], PCA9555_GPIO_STATE_HIGH);
    } else {
      pca9555_gpio_set_state(&s_output_leds[POWER_LED], PCA9555_GPIO_STATE_LOW);
    }
    s_last_power_state = get_power_info_power_state();
  }

  // Update left/right LED
  if (get_steering_info_input_lights() != s_last_lights_state) {
    switch (get_steering_info_input_lights()) {
      case EE_STEERING_LIGHTS_OFF_STATE:
        pca9555_gpio_set_state(&s_output_leds[LEFT_LED], PCA9555_GPIO_STATE_LOW);
        pca9555_gpio_set_state(&s_output_leds[RIGHT_LED], PCA9555_GPIO_STATE_LOW);
        break;
      case EE_STEERING_LIGHTS_LEFT_STATE:
        pca9555_gpio_set_state(&s_output_leds[LEFT_LED], PCA9555_GPIO_STATE_HIGH);
        break;
      case EE_STEERING_LIGHTS_RIGHT_STATE:
        pca9555_gpio_set_state(&s_output_leds[RIGHT_LED], PCA9555_GPIO_STATE_HIGH);
        break;
      default:
        break;
    }
    s_last_lights_state = get_steering_info_input_lights();
  }
}

void monitor_cruise_control() {
  // Check steering message for cc event (toggle/increase/decrease)
  // Update cc enabled based on brake/cc toggle
  uint8_t cc_info = get_steering_info_input_cc();
  bool new_cc_state = s_cc_enabled;
  if (get_drive_state() != DRIVE || get_pedal_output_brake_output()) {
    new_cc_state = false;
  } else {
    if (get_received_steering_info() && (cc_info & EE_STEERING_CC_TOGGLE_MASK)) {
      new_cc_state = !s_cc_enabled;
    }
  }

  // If a state change has occurred update values and indicator LED
  if (new_cc_state != s_cc_enabled) {
    if (new_cc_state) {
      // Store recent speed from MCI as initial cruise control speed
      float avg_speed = (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) / 2;
      float speed_kph = avg_speed * CONVERT_VELOCITY_TO_KPH;
      s_target_velocity = (unsigned int)speed_kph;
      pca9555_gpio_set_state(&s_output_leds[CRUISE_LED], PCA9555_GPIO_STATE_HIGH);
    } else {
      s_target_velocity = 0;
      pca9555_gpio_set_state(&s_output_leds[CRUISE_LED], PCA9555_GPIO_STATE_LOW);
    }
    s_cc_enabled = new_cc_state;
  }

  // Allow for updates to cruise control value if it is enabled
  if (s_cc_enabled) {
    if (get_received_steering_info() && (cc_info & EE_STEERING_CC_INCREASE_MASK)) {
      s_target_velocity++;
    }
    if (get_received_steering_info() && (cc_info & EE_STEERING_CC_DECREASE_MASK)) {
      s_target_velocity--;
    }
  }
}

void update_displays(void) {
  // Read data from CAN structs and update displays with those values
  float avg_speed = (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) / 2;
  float speed_kph = avg_speed * CONVERT_VELOCITY_TO_KPH;
  uint16_t batt_perc_val = get_battery_status_batt_perc();
  if (speed_kph >= 100) {
    seg_displays_set_int(&all_displays, (int)speed_kph, batt_perc_val, s_target_velocity);
  } else {
    seg_displays_set_float(&all_displays, speed_kph, batt_perc_val, s_target_velocity);
  }
}

void update_drive_output(uint32_t notif) {
  if (notify_check_event(&notif, POWER_BUTTON_EVENT)) {
    if (get_pedal_output_brake_output()) {
      set_cc_power_control_power_event(EE_CC_PWR_CTL_EVENT_BTN_AND_BRAKE);
    } else {
      set_cc_power_control_power_event(EE_CC_PWR_CTL_EVENT_BTN);
    }
  } else {
    set_cc_power_control_power_event(EE_CC_PWR_CTL_EVENT_NONE);
  }
  set_drive_output_cruise_control(s_cc_enabled);
  set_drive_output_target_velocity(s_target_velocity);
  set_drive_output_regen_braking(s_regen_braking);
  set_cc_power_control_hazard_enabled(s_hazard_state);
}

StatusCode dashboard_init(void) {
  Pca9555GpioSettings settings = {
    .direction = PCA9555_GPIO_DIR_OUT,
    .state = PCA9555_GPIO_STATE_LOW,
  };
  for (int i = 0; i < NUM_DRIVE_LED; i++) {
    status_ok_or_return(pca9555_gpio_init_pin(&s_output_leds[i], &settings));
  }
  seg_displays_init(&all_displays);
  pca9555_gpio_set_state(&s_output_leds[REGEN_LED], PCA9555_GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}
