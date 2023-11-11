#pragma once

#include "drive_fsm.h"
#include "i2c.h"
#include "pca9555_gpio_expander.h"
#include "pca9555_gpio_expander_defs.h"
#include "power_fsm.h"

#define PCA9555_REG_DEFAULT 0xFF

#define DRIVE_BTN_MASK 0x01
#define POWER_BTN_MASK 0x02
#define NEUTRAL_BTN_MASK 0x04
#define REVERSE_BTN_MASK 0x08

StatusCode get_button_press(void);
