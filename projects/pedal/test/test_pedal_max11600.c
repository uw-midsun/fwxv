#include "log.h"
#include "max11600.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_MAX11600_I2C_PORT I2C_PORT_2
#define REFERENCE_VOLTAGE_V 3.3
#define MAX11600_READ_ADDRESS 0xC9
#define MAX11600_WRITE_ADDRESS 0xC8

static Max11600Storage max_storage = { 0 };

void setup_test(void) {
  log_init();
  LOG_DEBUG("here");
  TEST_ASSERT_OK(max11600_init(&max_storage, TEST_MAX11600_I2C_PORT));
}

void test_max11600_init(void) {
  TEST_ASSERT_EQUAL(TEST_MAX11600_I2C_PORT, max_storage.i2c_port);
  TEST_ASSERT_EQUAL(MAX11600_READ_ADDRESS, max_storage.i2c_read_address);
  TEST_ASSERT_EQUAL(MAX11600_WRITE_ADDRESS, max_storage.i2c_write_address);
  for (int channel = 0; channel < NUM_MAX11600_CHANNELS; channel++) {
    TEST_ASSERT_EQUAL(0, max_storage.channel_readings[channel]);
  }
}

void test_max11600_read_raw(void) {
  uint8_t tx_data[3];
  tx_data[0] = tx_data[1] = tx_data[2] = 0x00;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_raw(&max_storage));
  TEST_ASSERT_EQUAL(0x00, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0x00, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0x00, max_storage.channel_readings[2]);
  tx_data[0] = 0xFF;
  tx_data[1] = 0xFF;
  tx_data[2] = 0xFF;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_raw(&max_storage));
  TEST_ASSERT_EQUAL(0xFF, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0xFF, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0xFF, max_storage.channel_readings[2]);
}

void test_max11600_read_converted(void) {}

/*
void test_max11600_read_converted(void) {
  for (int LSB_THROTTLE = 0; LSB_THROTTLE < 256; LSB_THROTTLE++) {
    for (int LSB_STEERING = 0; LSB_STEERING < 256; LSB_STEERING++) {
      for (int LSB_BRAKE = 0; LSB_BRAKE < 256; LSB_BRAKE++) {
        // Mock all possible combinations of LSD onto the I2C
        uint8_t tx_data[] = { LSB_THROTTLE, LSB_STEERING, LSB_BRAKE };
        i2c_write(TEST_MAX11600_I2C_PORT, MAX11600_WRITE_ADDRESS, tx_data, 3);
        // Read the converted data from the MAX11600
        TEST_ASSERT_OK(max11600_read_converted(&max_storage));
        for (int channel = 0; channel < 3; channel++) {
          // Calculate the expected converted value
          uint16_t expected_converted_value = 1000 * REFERENCE_VOLTAGE_V * tx_data[channel] / 256;
          TEST_ASSERT_EQUAL(expected_converted_value, max_storage.channel_readings[channel]);
        }
      }
    }
  }
}
*/

void teardown_test(void) {}
