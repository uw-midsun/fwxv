#pragma once
#define PROJECTS_PEDAL_INC_MAX11600_H_

#include <stdbool.h>

#include "gpio.h"
#include "i2c.h"

// Reference voltage for the IC, from the board schematic
#define REFERENCE_VOLTAGE_V 3.3

typedef enum {
  MAX11600_CHANNEL_0 = 0,
  MAX11600_CHANNEL_1,
  MAX11600_CHANNEL_2,
  MAX11600_CHANNEL_3,
  NUM_MAX11600_CHANNELS,
} MAX11600Channel;

typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_read_address;
  I2CAddress i2c_write_address;
  uint16_t channel_readings[NUM_MAX11600_CHANNELS];
} Max11600Storage;

// Initializes the storage struct with the I2C port
StatusCode max11600_init(Max11600Storage *storage, I2CPort i2c_port);

// Reads the raw data from the MAX11600's AIN channels and stores it into the storage struct
// Updates the storage readings with the raw values
StatusCode max11600_read_raw(Max11600Storage *storage);

// Reads the raw data from the MAX11600's AIN channels and stores it into the storage struct
// Updates the storage readings with the converted values (in mV)
StatusCode max11600_read_converted(Max11600Storage *storage);
