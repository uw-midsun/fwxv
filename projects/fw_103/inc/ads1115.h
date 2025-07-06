#pragma once

#include "gpio.h"
#include "i2c.h"
#include "tasks.h"

#define ALRT_EVENT 0

#define ADS1115_I2C_PORT 0

typedef enum {
  ADS1115_ADDR_GND = 0x48,  // 0b1001000
  ADS1115_ADDR_VDD = 0x49,  // 0b1001001
  ADS1115_ADDR_SDA = 0x4A,  // 0b1001010
  ADS1115_ADDR_SCL = 0x4B,  // 0b1001011
} ADS1115_Address;

typedef enum {
  ADS1115_CHANNEL_0 = 0,
  ADS1115_CHANNEL_1,
  ADS1115_CHANNEL_2,
  ADS1115_CHANNEL_3,
} ADS1115_Channel;

typedef enum {
  ADS1115_REG_CONVERSION = 0x00,
  ADS1115_REG_CONFIG,
  ADS1115_REG_LO_THRESH,
  ADS1115_REG_HI_THRESH,
} ADS1115_Reg;

typedef struct ADS1115_Config {
  I2CPort i2c_port;
  I2CAddress i2c_addr;
  Task *handler_task;
  GpioAddress *ready_pin;
} ADS1115_Config;

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin);

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel);

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading);

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading);
