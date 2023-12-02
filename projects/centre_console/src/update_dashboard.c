#include "update_dashboard.h"

#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "pca9555_gpio_expander.h"
#include "seg_display.h"

// Multiplication factor to convert CAN motor velocity (cm/s) into drive output velocity (mm/s)
#define CONVERT_VELOCITY 5

// Multiplication factor to convert CAN drive output velocity to kph
#define CONVERT_VELOCITY_TO_KPH 0.0036

// Multiplication Factor to convert CAN Velocity in 100 * m/s to kph
#define CONVERT_VELOCITY_TO_SPEED 0.018

static SegDisplay cc_display = CC_DISPLAY;
static SegDisplay speed_display = SPD_DISPLAY;
SegDisplay batt_perc_display = BATT_DISPLAY;

// Centre Console State Variables
static uint8_t s_drive_state;
static bool s_cc_enabled;
static bool s_regen_braking;
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
  NUM_DRIVE_LED,
} DriveLeds;

static Pca9555GpioAddress s_output_leds[NUM_DRIVE_LED] = {
  [POWER_LED] = CC_LED_POWER,   [HAZARD_LED] = HAZARD_LED_ADDR, [LEFT_LED] = LEFT_LED_ADDR,
  [RIGHT_LED] = RIGHT_LED_ADDR, [CRUISE_LED] = CRUISE_LED_ADDR, [REGEN_LED] = REGEN_LED_ADDR
};

void update_indicators(uint32_t notif) {
  // Update hazard light
  if (notify_check_event(&notif, HAZARD_BUTTON_EVENT)) {
    if (s_regen_braking) {
      s_regen_braking = false;
      pca9555_gpio_set_state(&s_output_leds[HAZARD_LED], PCA9555_GPIO_STATE_LOW);
    } else {
      s_regen_braking = false;
      pca9555_gpio_set_state(&s_output_leds[HAZARD_LED], PCA9555_GPIO_STATE_HIGH);
    }
  }

  // Update regen light
  if (notify_check_event(&notif, REGEN_BUTTON_EVENT)) {
    if (s_regen_braking) {
      s_regen_braking = false;
      pca9555_gpio_set_state(&s_output_leds[REGEN_LED], PCA9555_GPIO_STATE_LOW);
    } else {
      s_regen_braking = false;
      pca9555_gpio_set_state(&s_output_leds[REGEN_LED], PCA9555_GPIO_STATE_HIGH);
    }
  }

  // Update power btn light if power_state has changed
  if (get_power_info_power_state() != s_last_power_state) {
    if (get_power_info_power_state() == EE_POWER_ON_STATE ||
        get_power_info_power_state() == EE_POWER_DRIVE_STATE) {
      pca9555_gpio_set_state(&s_output_leds[POWER_LED], PCA9555_GPIO_STATE_HIGH);
    } else {
      pca9555_gpio_set_state(&s_output_leds[POWER_LED], PCA9555_GPIO_STATE_HIGH);
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
    if (cc_info & EE_STEERING_CC_TOGGLE_MASK) {
      new_cc_state = !s_cc_enabled;
    }
  }

  // If a state change has occurred update values and indicator LED
  if (new_cc_state != s_cc_enabled) {
    if (s_cc_enabled) {
      // Store recent speed from MCI as initial cruise control speed
      unsigned int convert_velocity = CONVERT_VELOCITY;
      float converted_val =
          (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) * CONVERT_VELOCITY;
      s_target_velocity = (unsigned int)converted_val;
      pca9555_gpio_set_state(&s_output_leds[CRUISE_LED], PCA9555_GPIO_STATE_HIGH);
    } else {
      s_target_velocity = 0;
      pca9555_gpio_set_state(&s_output_leds[CRUISE_LED], PCA9555_GPIO_STATE_LOW);
    }
  }

  // Allow for updates to cruise control value if it is enabled
  if (s_cc_enabled) {
    if (cc_info & EE_STEERING_CC_INCREASE_MASK) {
      s_target_velocity =
          ((s_target_velocity * CONVERT_VELOCITY_TO_KPH) + 1) / CONVERT_VELOCITY_TO_KPH;
    }
    if (cc_info & EE_STEERING_CC_DECREASE_MASK) {
      s_target_velocity =
          ((s_target_velocity * CONVERT_VELOCITY_TO_KPH) - 1) / CONVERT_VELOCITY_TO_KPH;
    }
  }
}

void update_displays(void) {
  seg_display_init(&cc_display);
  seg_display_init(&speed_display);
  seg_display_init(&batt_perc_display);

  // Read data from CAN structs and update displays with those values
  float speed_kph =
      (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) * CONVERT_VELOCITY;
  uint16_t batt_perc_val = get_battery_status_batt_perc();
  seg_display_set_int(&cc_display, s_target_velocity);
  if (speed_kph >= 100) {
    seg_display_set_int(&speed_display, (int)speed_kph);
  } else {
    seg_display_set_float(&speed_display, speed_kph);
  }
  seg_display_set_int(&batt_perc_display, batt_perc_val);
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
  set_drive_output_drive_state(s_drive_state);
  set_drive_output_cruise_control(s_cc_enabled);
  set_drive_output_target_velocity(s_target_velocity);
  set_drive_output_regen_braking(s_regen_braking);
}

StatusCode dashboard_init(void) {
  Pca9555GpioSettings settings = {
    .direction = PCA9555_GPIO_DIR_OUT,
    .state = PCA9555_GPIO_STATE_LOW,
  };
  for (int i = 0; i < NUM_DRIVE_LED; i++) {
    status_ok_or_return(pca9555_gpio_init_pin(&s_output_leds[i], &settings));
  }
  return STATUS_CODE_OK;
}
