#pragma once
#include "bts_load_switch.h"
#include "pca9555_gpio_expander.h"
#include "gpio.h"
#include "i2c.h"

#define I2C_SDA_PORT GPIO_PORT_B
#define I2C_SDA_PIN 9
#define I2C_SCL_PORT GPIO_PORT_B
#define I2C_SCL_PIN 8

#define I2C_ADDRESS_1 0x24
#define I2C_ADDRESS_2 0x26

GpioAddress i2c_sda = {
  .port = I2C_SDA_PORT,
  .pin = I2C_SDA_PIN
};

GpioAddress i2c_scl = {
  .port = I2C_SCL_PORT,
  .pin = I2C_SCL_PIN
};

I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD, 
  .sda = {
    .port = I2C_SDA_PORT,
    .pin = I2C_SDA_PIN
  },
  .scl = {
  .port = I2C_SCL_PORT,
  .pin = I2C_SCL_PIN
  }
};
