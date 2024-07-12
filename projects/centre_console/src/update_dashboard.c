#include "update_dashboard.h"

#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "pca9555_gpio_expander.h"
#include "pwm.h"
#include "seg_display.h"

// Multiplication factor to convert CAN drive output velocity (cm/s) to kph
#define CONVERT_VELOCITY_TO_KPH 0.036

SegDisplay all_displays = ALL_DISPLAYS;
GpioAddress bps_led = BPS_LED_ADDR;

// Centre Console State Variables
static bool s_cc_enabled;
static float s_regen_braking;
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
  AUX_WARNING_LED,
  NUM_DRIVE_LED,
} DriveLeds;

static Pca9555GpioAddress s_output_leds[NUM_DRIVE_LED] = {
  [POWER_LED] = POWER_LED_ADDR,
  [HAZARD_LED] = HAZARD_LED_ADDR,
  [LEFT_LED] = LEFT_LED_ADDR,
  [RIGHT_LED] = RIGHT_LED_ADDR,
  [CRUISE_LED] = CRUISE_LED_ADDR,
  [REGEN_LED] = REGEN_LED_ADDR,
  [AUX_WARNING_LED] = AUX_WARNING_LED_ADDR,
};

static float prv_regen_calc(uint16_t max_cell_v, uint16_t batt_soc) {
  return 1.0f;
  if (batt_soc >= 90) {
    return (float)(100.0f - batt_soc) / 100.0f;
  } else {
    return (MAX_VOLTAGE - (max_cell_v < MIN_VOLTAGE ? MIN_VOLTAGE : max_cell_v)) /
           (MAX_VOLTAGE - MIN_VOLTAGE);
  }
}

void update_indicators(uint32_t notif) {
  // Update hazard light
  if (notify_check_event(&notif, HAZARD_BUTTON_EVENT)) {
    if (!s_hazard_state) {
      s_hazard_state = true;
      pca9555_gpio_set_state(&s_output_leds[HAZARD_LED], PCA9555_GPIO_STATE_HIGH);
    }
  } else {
    s_hazard_state = false;
    pca9555_gpio_set_state(&s_output_leds[HAZARD_LED], PCA9555_GPIO_STATE_LOW);
  }
  // Update regen light
  if (notify_check_event(&notif, REGEN_BUTTON_EVENT)) {
    uint16_t batt_voltage = get_battery_info_max_cell_v();  // Gets max voltage out of all cells
    uint16_t batt_current = get_battery_vt_current();
    // solar current + regen current <= 27 AMPS
    // regen current shouldnt push cell above 4.2 V
    if (!s_regen_braking && get_battery_vt_batt_perc() < MAX_VOLTAGE) {
      s_regen_braking = prv_regen_calc(get_battery_info_max_cell_v(), get_battery_vt_batt_perc());
      pca9555_gpio_set_state(&s_output_leds[REGEN_LED], PCA9555_GPIO_STATE_HIGH);
    } else {
      s_regen_braking = 0.0f;
      pca9555_gpio_set_state(&s_output_leds[REGEN_LED], PCA9555_GPIO_STATE_LOW);
    }
  }

  // Update left/right LED
  if (g_tx_struct.cc_steering_input_lights != s_last_lights_state) {
    switch (g_tx_struct.cc_steering_input_lights) {
      case EE_STEERING_LIGHTS_OFF_STATE:
        pca9555_gpio_set_state(&s_output_leds[LEFT_LED], PCA9555_GPIO_STATE_LOW);
        pca9555_gpio_set_state(&s_output_leds[RIGHT_LED], PCA9555_GPIO_STATE_LOW);
        break;
      case EE_STEERING_LIGHTS_LEFT_STATE:
        pca9555_gpio_set_state(&s_output_leds[LEFT_LED], PCA9555_GPIO_STATE_HIGH);
        pca9555_gpio_set_state(&s_output_leds[RIGHT_LED], PCA9555_GPIO_STATE_LOW);
        break;
      case EE_STEERING_LIGHTS_RIGHT_STATE:
        pca9555_gpio_set_state(&s_output_leds[RIGHT_LED], PCA9555_GPIO_STATE_HIGH);
        pca9555_gpio_set_state(&s_output_leds[LEFT_LED], PCA9555_GPIO_STATE_LOW);
        break;
      default:
        break;
    }
    s_last_lights_state = g_tx_struct.cc_steering_input_lights;
  }

  // Update Aux warning LED
  if (get_pd_status_fault_bitset()) {
    pwm_set_dc(PWM_TIMER_1, 50, 3, true);
    pca9555_gpio_set_state(&s_output_leds[AUX_WARNING_LED], PCA9555_GPIO_STATE_HIGH);
  } else {
    // PWM will not stop, driver will pull over and diagnose issue
    pca9555_gpio_set_state(&s_output_leds[AUX_WARNING_LED], PCA9555_GPIO_STATE_LOW);
  }

  if (get_battery_status_fault() & get_pd_status_bps_persist() & (1 << 15)) {
    pwm_set_dc(PWM_TIMER_1, 100, 3, true);
    gpio_set_state(&bps_led, GPIO_STATE_LOW);
  } else if ((get_battery_status_fault() & get_pd_status_bps_persist() & (1 << 14)) ||
             get_pd_status_power_state() == EE_POWER_FAULT_STATE) {
    pwm_set_dc(PWM_TIMER_1, 10, 3, true);
    gpio_set_state(&bps_led, GPIO_STATE_LOW);
  } else {
    // PWM will not stop, driver will pull over and diagnose issue
    gpio_set_state(&bps_led, GPIO_STATE_HIGH);
  }
}

void monitor_cruise_control() {
  // Check steering message for cc event (toggle/increase/decrease)
  // Update cc enabled based on brake/cc toggle
  uint8_t cc_info = g_tx_struct.cc_steering_input_cc;
  bool new_cc_state = s_cc_enabled;
  if (g_tx_struct.cc_info_drive_state != DRIVE || g_tx_struct.cc_pedal_brake_output) {
    new_cc_state = false;
  } else {
    if (cc_info & EE_STEERING_CC_TOGGLE_MASK) {
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
    if (cc_info & EE_STEERING_CC_INCREASE_MASK) {
      s_target_velocity += 1000;  // 1000 for testing should actually be 1
    }
    if (cc_info & EE_STEERING_CC_DECREASE_MASK) {
      s_target_velocity -= 1000;
    }
  }
}

void update_drive_output() {
  set_cc_info_cruise_control(s_cc_enabled);
  set_cc_info_target_velocity(s_target_velocity);
  memcpy(&g_tx_struct.cc_regen_percent, &s_regen_braking, sizeof(s_regen_braking));
  set_cc_info_hazard_enabled(s_hazard_state);
}

TASK(update_displays, TASK_MIN_STACK_SIZE) {
  seg_displays_init(&all_displays);
  while (true) {
    float avg_speed = (abs((int16_t)get_motor_velocity_velocity_l()) +
                       abs((int16_t)get_motor_velocity_velocity_r())) /
                      2.0f;
    float speed_kph = avg_speed * CONVERT_VELOCITY_TO_KPH;
    uint16_t batt_perc_val = get_battery_vt_batt_perc();
    uint16_t aux_battery_voltage = get_battery_status_aux_batt_v();
    if (speed_kph >= 100) {
      seg_displays_set_int(&all_displays, (int)speed_kph, batt_perc_val, aux_battery_voltage);
    } else {
      seg_displays_set_float(&all_displays, speed_kph, batt_perc_val, aux_battery_voltage);
    }
  }
}

StatusCode dashboard_init(void) {
  Pca9555GpioSettings settings = {
    .direction = PCA9555_GPIO_DIR_OUT,
    .state = PCA9555_GPIO_STATE_LOW,
  };
  for (int i = 0; i < NUM_DRIVE_LED; i++) {
    status_ok_or_return(pca9555_gpio_init_pin(&s_output_leds[i], &settings));
  }
  gpio_init_pin(&bps_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  return STATUS_CODE_OK;
}

StatusCode display_init(void) {
  return tasks_init_task(update_displays, TASK_PRIORITY(2), NULL);
}
