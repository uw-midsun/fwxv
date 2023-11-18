#pragma once

#include "drive_fsm.h"
#include "i2c.h"
#include "pca9555_gpio_expander.h"
#include "pca9555_gpio_expander_defs.h"
#include "power_fsm.h"
#include "update_dashboard.h"

#define PCA9555_REG_DEFAULT 0xFF

// Masks for button inputs based on a 16-bit pca9555 register value
typedef enum ConsoleButtonMask {
  REGEN_BTN_MASK = 0x040,   // 0_6
  HAZARD_BTN_MASK = 0x080,  // 0_7
  DRIVE_BTN_MASK = 0x100,   // 1_0
  POWER_BTN_MASK = 0x200,   // 1_1
  NEUTRAL_BTN_MASK = 0x400, // 1_2
  REVERSE_BTN_MASK = 0x800, // 1_3
} ConsoleButtonMask;

StatusCode get_button_press(void);
