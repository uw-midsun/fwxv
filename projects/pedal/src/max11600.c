#include "max11600.h"

#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"

// Reference Voltage:       000
// External Clock:          1
// Unipolar:                0
// Resetting Configuration: 0
// Don't care:              0
// Binary:                  10001000
#define MAX11600_SETUP_BYTE 0x88;

// Scanning Pattern:        00
// Channel Select:          AIN2
// Single-ended:            1
// Binary:                  00000101
#define MAX11600_CONFIG_BYTE 0x05;

// Read Mask
#define MAX11600_READ_ADDRESS 0xC9;
// Write Mask
#define MAX11600_WRITE_ADDRESS 0xC8;

static const uint8_t i2c_read_address = MAX11600_READ_ADDRESS;
static const uint8_t i2c_write_address = MAX11600_WRITE_ADDRESS;

// We assume that the channels will always be read in the order AIN0 -> AIN2
StatusCode max11600_init(Max11600Storage *storage, I2CPort i2c_port) {
  StatusCode status;
  storage->i2c_port = i2c_port;
  storage->i2c_read_address = i2c_read_address;
  storage->i2c_write_address = i2c_write_address;

  // send config and setup byte
  // initialize/send correct setup byte with proper scanning pattern to scan from AIN0 -> AIN2
  uint8_t setup_byte = MAX11600_SETUP_BYTE;
  uint8_t config_byte = MAX11600_CONFIG_BYTE;
  status = i2c_write(i2c_port, i2c_write_address, &setup_byte, 1);
  if (status == STATUS_CODE_OK) {
    status = i2c_write(i2c_port, i2c_write_address, &config_byte, 1);
  }
  return status;
}

StatusCode max11600_read_raw(Max11600Storage *storage) {
  StatusCode status;
  uint8_t rx_data[3];
  // read in the data, and update channel readings to read AIN0 -> AIN2 (scan0 = scan1 = 0)
  status = i2c_read(storage->i2c_port, storage->i2c_read_address, rx_data, 3);
  storage->channel_readings[0] = rx_data[0];
  storage->channel_readings[1] = rx_data[1];
  storage->channel_readings[2] = rx_data[2];
  return status;
}

StatusCode max11600_read_converted(Max11600Storage *storage) {
  StatusCode status;
  status = max11600_read_raw(storage);
  storage->channel_readings[0] *= 1000 * REFERENCE_VOLTAGE_V / 256;
  storage->channel_readings[1] *= 1000 * REFERENCE_VOLTAGE_V / 256;
  storage->channel_readings[2] *= 1000 * REFERENCE_VOLTAGE_V / 256;
  return status;
}
