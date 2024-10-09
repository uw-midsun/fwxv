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
  AFE_WARNING_LED,
  NUM_DRIVE_LED,
} DriveLeds;

static Pca9555GpioAddress s_output_leds[NUM_DRIVE_LED] = {
  [POWER_LED] = POWER_LED_ADDR,
  [HAZARD_LED] = HAZARD_LED_ADDR,
  [LEFT_LED] = LEFT_LED_ADDR,
  [RIGHT_LED] = RIGHT_LED_ADDR,
  [CRUISE_LED] = CRUISE_LED_ADDR,
  [REGEN_LED] = REGEN_LED_ADDR,
  [AFE_WARNING_LED] = AFE_WARNING_LED_ADDR,
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
  if (get_battery_status_afe_status()) {
    pca9555_gpio_set_state(&s_output_leds[AFE_WARNING_LED], PCA9555_GPIO_STATE_HIGH);
  } else {
    // PWM will not stop, driver will pull over and diagnose issue
    pca9555_gpio_set_state(&s_output_leds[AFE_WARNING_LED], PCA9555_GPIO_STATE_LOW);
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

void update_drive_output() {
  memcpy(&g_tx_struct.cc_regen_percent, &s_regen_braking, sizeof(s_regen_braking));
  set_cc_info_hazard_enabled(s_hazard_state);
}

TASK(update_displays, TASK_MIN_STACK_SIZE) {
  seg_displays_init(&all_displays);
  float car_vel = 0;
  uint16_t batt_fault = 0;
  while (true) {
    if (get_battery_status_fault()) {
      batt_fault = (get_battery_status_fault()) & (~(0b11 << 14));
    } else {
      float avg_speed = (abs((int16_t)get_motor_velocity_velocity_l()) +
                         abs((int16_t)get_motor_velocity_velocity_r())) /
                        2.0f;
    
      car_vel = (avg_speed * 60 * MATH_PI * (WHEEL_DIAMETER_CM / CM_TO_INCHES)) / MILES_TO_INCHES;
      if (car_vel >= 100.0f) {
        car_vel = 99.9f;
      }
    }
    int16_t batt_current = (int16_t)(get_battery_vt_current());
    batt_current = abs(batt_current / 100);
    uint16_t aux_battery_voltage = get_battery_status_aux_batt_v();
    if (get_battery_status_fault()) {
      seg_displays_set_int(&all_displays, batt_fault, (uint16_t)batt_current, aux_battery_voltage);
    } else if (car_vel >= 100) {
      seg_displays_set_int(&all_displays, (int)car_vel, (uint16_t)batt_current,
                           aux_battery_voltage);
    } else {
      seg_displays_set_float(&all_displays, car_vel, (uint16_t)batt_current, aux_battery_voltage);
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
