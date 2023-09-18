#include "current_sense.h"

#include <stdint.h>

#include "gpio.h"
#include "i2c.h"
#include "solar_sense_setters.h"
#include "status.h"

#define LTC2451_I2C_ADDR 0x14

#define ADC_REFERENCE_VOLTAGE 3.3
#define ADC_RESOLUTION 65535

#define CURRENT_THRESHOLD 100  // TO-DO: Ask HW team
#define CURRENT_SENSOR_SENSITIVITY 0.066

static GpioAddress relay_en_address = { GPIO_PORT_B, 4 };      // Enable pin
static GpioAddress relay_status_address = { GPIO_PORT_A, 6 };  // Status pin

static GpioAddress *relay_en_gpio = &relay_en_address;
static GpioAddress *relay_status_gpio = &relay_status_address;

static GpioState relay_status;

// LTC2451 ADC Possible conversion speeds
#define CURRENT_SENSE_ADC_30_HZ 0x80;
#define CURRENT_SENSE_ADC_60_HZ 0x00;

// Set conversion speed to either 30Hz or 60Hz defined above
StatusCode current_sense_init(uint8_t conversion_speed) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 8 },
    .sda = { .port = GPIO_PORT_B, .pin = 9 },
  };

  status_ok_or_return(i2c_init(I2C_PORT_1, &i2c_settings));

  // Write to the LTC2451 to set conversion speed
  status_ok_or_return(i2c_write(I2C_PORT_1, LTC2451_I2C_ADDR, &conversion_speed, 1));

  // Initialize GPIOs for relay
  status_ok_or_return(gpio_init_pin(relay_en_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(gpio_init_pin(relay_status_gpio, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW));
  status_ok_or_return(gpio_set_state(relay_status_gpio, GPIO_STATE_LOW));

  return STATUS_CODE_OK;
}

// ADC reading and conversion
StatusCode current_sense_main_cycle() {
  uint8_t read_bytes[2];
  status_ok_or_return(i2c_read(I2C_PORT_1, LTC2451_I2C_ADDR, read_bytes, 2));

  // Convert read bytes to single value
  uint16_t adc_reading = (read_bytes[0] << 8) | read_bytes[1];

  uint16_t voltage_measured = (uint16_t)((ADC_REFERENCE_VOLTAGE * adc_reading) / ADC_RESOLUTION);
  uint16_t current_measured = (uint16_t)(voltage_measured / CURRENT_SENSOR_SENSITIVITY);

  // Open relay on fault
  if (current_measured > CURRENT_THRESHOLD) {
    current_sense_relay_set(GPIO_STATE_HIGH);
  }

  // Send info to CAN
  set_current_sense_current(current_measured);
  set_current_sense_voltage(voltage_measured);
  set_current_sense_relay_status(relay_status);

  return STATUS_CODE_OK;
}

// Set state to: GPIO_LOW -> Relay close; GPIO_HIGH -> Relay open
StatusCode current_sense_relay_set(GpioState state) {
  status_ok_or_return(gpio_set_state(relay_en_gpio, state));
  status_ok_or_return(gpio_get_state(relay_status_gpio, &relay_status));

  return STATUS_CODE_OK;
}
