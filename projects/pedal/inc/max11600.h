#ifndef PROJECTS_PEDAL_INC_MAX11600_H_
#define PROJECTS_PEDAL_INC_MAX11600_H_

#include <stdbool.h>

#include "gpio.h"
#include "i2c.h"

// Reference Voltage: 000
// External Clock: 1
// Unipolar: 0
// Resetting Configuration: 0
// Don't care: 0
static const uint8_t max11600_setup_byte = 0b10001000;

// Scanning Pattern: 00
// Channel Select: AIN2
// Single-ended: 1
static const uint8_t max11600_config_byte = 0b00000101;

// Read Mask
static const uint8_t max11600_read_address = 0b11001001;
// Write Mask
static const uint8_t max11600_write_address = 0b11001000;

#define NUM_MAX11600_CHANNELS 3
#define REFERENCE_VOLTAGE 5

typedef struct {
  I2CPort i2c_port;  // I2C_PORT_1 or I2C_PORT_2
  I2CAddress i2c_read_address;
  I2CAddress i2c_write_address;
  int16_t channel_readings[NUM_MAX11600_CHANNELS];
} Max11600Storage;

// i2c_write(I2C_PORT_1, MAX11600_WRITE_ADDRESS, MAX11600_SETUP_BYTE, 1);
StatusCode max11600_init(Max11600Storage *Storage, I2CPort i2c_port, I2CAddress i2c_read_address,
                         I2CAddress i2c_write_address);

StatusCode max11600_read_raw(Max11600Storage *Storage);

StatusCode max11600_read_converted(Max11600Storage *Storage);

#endif  // PROJECTS_PEDAL_INC_MAX11600_H_
