#include "max11600.h"

#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"

// We assume that the channels will always be read in the order AIN0 -> AIN2
StatusCode max11600_init(Max11600Storage *Storage, I2CPort i2c_port, I2CAddress i2c_read_address,
                         I2CAddress i2c_write_address) {
  StatusCode status;
  Storage->i2c_port = i2c_port;
  Storage->i2c_read_address = i2c_read_address;
  Storage->i2c_write_address = i2c_write_address;

  // send config and setup byte
  // initialize/send correct setup byte with proper scanning pattern to scan from AIN0 -> AIN2
  status = i2c_write(i2c_port, i2c_write_address, &max11600_setup_byte, 1);
  if (status == STATUS_CODE_OK) {
    status = i2c_write(i2c_port, i2c_write_address, &max11600_config_byte, 1);
  }
  return status;
}

StatusCode max11600_read_raw(Max11600Storage *Storage) {
  StatusCode status;
  uint8_t rx_data[3];
  // read in the data, and update channel readings to read AIN0 -> AIN2 (scan0 = scan1 = 0)
  status = i2c_read(Storage->i2c_port, Storage->i2c_read_address, rx_data, 3);
  Storage->channel_readings[0] = rx_data[0];
  Storage->channel_readings[1] = rx_data[1];
  Storage->channel_readings[2] = rx_data[2];
  return status;
}

StatusCode max11600_read_converted(Max11600Storage *Storage) {
  StatusCode status;
  status = max11600_read_raw(Storage);
  Storage->channel_readings[0] *= REFERENCE_VOLTAGE / 256 * 1000;
  Storage->channel_readings[1] *= REFERENCE_VOLTAGE / 256 * 1000;
  Storage->channel_readings[2] *= REFERENCE_VOLTAGE / 256 * 1000;
  return status;
}
