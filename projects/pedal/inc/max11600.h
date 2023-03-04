#pragma once
#define PROJECTS_PEDAL_INC_MAX11600_H_

#include <stdbool.h>

#include "gpio.h"
#include "i2c.h"

#define NUM_MAX11600_CHANNELS 3
#define REFERENCE_VOLTAGE 3.3
typedef struct {
  I2CPort i2c_port;  // I2C_PORT_1 or I2C_PORT_2
  I2CAddress i2c_read_address;
  I2CAddress i2c_write_address;
  uint8_t channel_readings[NUM_MAX11600_CHANNELS];
} Max11600Storage;

// Initializes the storage pointers with the i2c port and addresses needed for the driver, and sets
// up the MAX11600 ADC
StatusCode max11600_init(Max11600Storage *storage, I2CPort i2c_port, I2CAddress i2c_read_address,
                         I2CAddress i2c_write_address);

// Reads the raw data from the MAX11600's AIN channels and stores it into the storage pointer
// Updates the storage readings with the raw values
StatusCode max11600_read_raw(Max11600Storage *storage);

// Reads the raw data from the MAX11600's AIN channels and stores it into the storage pointer
// Updates the storage readings with the converted values (in mV)
StatusCode max11600_read_converted(Max11600Storage *storage);
