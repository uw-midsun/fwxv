#include "ads1115.h"  // Include the header for ADS1115 functions and types
#include "i2c.h"      // Include I2C communication functions
#include "log.h"      // Include logging functions

// Initializes the ADS1115 ADC with fixed settings
StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  
  // Set the I2C port for the ADC
  config->i2c_port = ADS1115_I2C_PORT;

  // Set the I2C address for the ADC
  config->i2c_addr = i2c_addr;

  // Set the ready pin (optional)
  config->ready_pin = ready_pin;

  // Set the handler task to NULL (not using interrupts)
  config->handler_task = NULL;

  // This value sets up the ADS1115's config register (16 bits)
  uint16_t config_value = 0;         // Start with all bits cleared
  config_value |= (1 << 15);         // OS: start a conversion (bit 15)
  config_value |= (0b000 << 12);     // MUX: AIN0 vs AIN1 (differential, bits 14-12))
  config_value |= (0b001 << 9);      // PGA: +/-4.096V range (bits 11-9)
  config_value |= (0 << 8);          // MODE: continuous conversion (bit 8)
  config_value |= (0b100 << 5);      // DR: 128 samples per second (bits 7-5)
  config_value |= (0 << 4);          // COMP_MODE: traditional comparator (bit 4)
  config_value |= (0 << 3);          // COMP_POL: active low (bit 3)
  config_value |= (0 << 2);          // COMP_LAT: non-latching (bit 2)
  config_value |= (0b11 << 0);       // COMP_QUE: disable comparator (bits 1-0)

  // Write the config to the ADC using I2C
  return i2c_write_reg(
    config->i2c_port,            // Which I2C port to use
    config->i2c_addr,            // The I2C address of the ADC
    ADS1115_REG_CONFIG,          // The register to write to (config register)
    (uint8_t *)&config_value,    // Pointer to the config value (as bytes)
    sizeof(config_value));       // Number of bytes to write (2 bytes)
}

// Reads a raw 16-bit ADC value (from conversion register)
StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading) {

  (void)channel;  // Ignore the channel parameter since we always read the conversion register

  return i2c_read_reg(
    config->i2c_port,  // Which I2C port to use
    config->i2c_addr,  // The I2C address of the ADC
                      
    ADS1115_REG_CONVERSION,    // The register to read (conversion register)
    (uint8_t *)reading,         // Where to store the result (as bytes)
    sizeof(*reading));         // Number of bytes to read (2 bytes)
}

// Converts the raw ADC value to a voltage (float)
StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  uint16_t raw = 0;  // Variable to store the raw ADC value

  // First read the raw ADC value
  StatusCode status = ads1115_read_raw(config, channel, &raw);  // Get the raw value from the ADC
  if (status != STATUS_CODE_OK) {   // If there was an error reading
    return status;                  // Return the error code
  }

  // Convert to voltage in range -2.048V to +2.048V
  // Formula: scale raw value (0 to 65535) to voltage range (-2.048V to +2.048V; total range = 4.096V)
  *reading = ((float)raw / 65535.0f) * 4.096f - 2.048f;

  return STATUS_CODE_OK;
}
