#include "can.h"
#include "gpio.h"
#include "log.h"
#include "pca9555_gpio_expander.h"
#include "power_fsm.h"
#include "seg_display.h"

#define NUM_DRIVE_LED 5

typedef enum DriveLeds {
  HAZARD_LED = 0,
  LEFT_LED,
  RIGHT_LED,
  CRUISE_LED,
  REGEN_LED,
} DriveLeds;

static Pca9555GpioAddress s_drive_btn_leds[NUM_DRIVE_LED] = {
  [HAZARD_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_4 },
  [LEFT_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO1_7 },
  [RIGHT_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO1_6 },
  [CRUISE_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_4 },
  [REGEN_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_5 }
};

StatusCode update_indicators(bool cruise_control_toggle, bool hazard_info,
                             bool regen_braking_toggle, bool left_signal_toggle,
                             bool right_signal_toggle) {
  // BOOL PARAMETERS
  // Cruise control: cruise_control_toggle
  // regen braking: regen_braking toggle
  // Left signal: left_signal_toggle
  // Right Signal: tight_signal_toggle
  // Hazard: hazard_info

  if (cruise_control_toggle) {
    pca9555_gpio_set_state(&s_drive_btn_leds[CRUISE_LED], PCA9555_GPIO_STATE_HIGH);
  } else {
    pca9555_gpio_set_state(&s_drive_btn_leds[CRUISE_LED], PCA9555_GPIO_STATE_LOW);
  }

  if (hazard_info) {
    pca9555_gpio_set_state(&s_drive_btn_leds[HAZARD_LED], PCA9555_GPIO_STATE_HIGH);
  } else {
    pca9555_gpio_set_state(&s_drive_btn_leds[HAZARD_LED], PCA9555_GPIO_STATE_LOW);
  }

  if (regen_braking_toggle) {
    pca9555_gpio_set_state(&s_drive_btn_leds[REGEN_LED], PCA9555_GPIO_STATE_HIGH);
  } else {
    pca9555_gpio_set_state(&s_drive_btn_leds[REGEN_LED], PCA9555_GPIO_STATE_LOW);
  }

  if (left_signal_toggle) {
    pca9555_gpio_set_state(&s_drive_btn_leds[LEFT_LED], PCA9555_GPIO_STATE_HIGH);
  } else {
    pca9555_gpio_set_state(&s_drive_btn_leds[LEFT_LED], PCA9555_GPIO_STATE_LOW);
  }

  if (right_signal_toggle) {
    pca9555_gpio_set_state(&s_drive_btn_leds[RIGHT_LED], PCA9555_GPIO_STATE_HIGH);
  } else {
    pca9555_gpio_set_state(&s_drive_btn_leds[RIGHT_LED], PCA9555_GPIO_STATE_LOW);
  }
}
