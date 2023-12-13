#pragma once
#include "bts_load_switch.h"
#include "gpio.h"
#include "i2c.h"
#include "pca9555_gpio_expander.h"
#include "pin_defs.h"

#define CHECK_EQUAL(actual, expected)                      \
  ({                                                       \
    __typeof__(actual) a = (actual);                       \
    __typeof__(actual) e = (__typeof__(actual))(expected); \
    if (a != e) {                                          \
      LOG_DEBUG("Equal check failed!\n");                  \
    }                                                      \
  })

I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = PD_I2C_SDA,
  .scl = PD_I2C_SCL,
};
