#include "i2c.h"
#include "log.h"
#include "max11600.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_MAX11600_I2C_PORT I2C_PORT_2
#define REFERENCE_VOLTAGE_V 3.3
#define MAX11600_READ_ADDRESS 0xC9
#define MAX11600_WRITE_ADDRESS 0xC8

static Max11600Storage max_storage = { 0 };
static I2CSettings i2c_settings = {
  .speed = I2C_SPEED_FAST,
  .scl = { .port = GPIO_PORT_B, .pin = 10 },
  .sda = { .port = GPIO_PORT_B, .pin = 11 },
};

void setup_test(void) {
  log_init();
  LOG_DEBUG("here");
  i2c_init(TEST_MAX11600_I2C_PORT, &i2c_settings);
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
  tx_data[0] = 0x00;
  tx_data[1] = 0x00;
  tx_data[2] = 0x00;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_raw(&max_storage));
  TEST_ASSERT_EQUAL(0x00, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0x00, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0x00, max_storage.channel_readings[2]);
  tx_data[0] = 0x7D;
  tx_data[1] = 0x45;
  tx_data[2] = 0x4D;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_raw(&max_storage));
  TEST_ASSERT_EQUAL(0x7D, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0x45, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0x4D, max_storage.channel_readings[2]);
  tx_data[0] = 0x97;
  tx_data[1] = 0xe0;
  tx_data[2] = 0x43;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_raw(&max_storage));
  TEST_ASSERT_EQUAL(0x97, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0xe0, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0x43, max_storage.channel_readings[2]);
  tx_data[0] = 0x2d;
  tx_data[1] = 0xd3;
  tx_data[2] = 0x1f;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_raw(&max_storage));
  TEST_ASSERT_EQUAL(0x2d, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0xd3, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0x1f, max_storage.channel_readings[2]);
  tx_data[0] = 0xfd;
  tx_data[1] = 0xbf;
  tx_data[2] = 0xa4;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_raw(&max_storage));
  TEST_ASSERT_EQUAL(0xfd, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0xbf, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0xa4, max_storage.channel_readings[2]);
}

void test_max11600_read_converted(void) {
  uint8_t tx_data[3];
  tx_data[0] = 0x00;
  tx_data[1] = 0x00;
  tx_data[2] = 0x00;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_converted(&max_storage));
  TEST_ASSERT_EQUAL(0, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(0, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(0, max_storage.channel_readings[2]);
  tx_data[0] = 0x7D;
  tx_data[1] = 0x45;
  tx_data[2] = 0x4D;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_converted(&max_storage));
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0x7D / 256, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0x45 / 256, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0x4D / 256, max_storage.channel_readings[2]);
  tx_data[0] = 0x97;
  tx_data[1] = 0xe0;
  tx_data[2] = 0x43;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_converted(&max_storage));
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0x97 / 256, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0xe0 / 256, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0x43 / 256, max_storage.channel_readings[2]);
  tx_data[0] = 0x2d;
  tx_data[1] = 0xd3;
  tx_data[2] = 0x1f;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_converted(&max_storage));
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0x2d / 256, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0xd3 / 256, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0x1f / 256, max_storage.channel_readings[2]);
  tx_data[0] = 0xfd;
  tx_data[1] = 0xbf;
  tx_data[2] = 0xa4;
  i2c_set_data(TEST_MAX11600_I2C_PORT, tx_data, 3);
  TEST_ASSERT_OK(max11600_read_converted(&max_storage));
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0xfd / 256, max_storage.channel_readings[0]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0xbf / 256, max_storage.channel_readings[1]);
  TEST_ASSERT_EQUAL(1000 * REFERENCE_VOLTAGE_V * 0xa4 / 256, max_storage.channel_readings[2]);
}

void teardown_test(void) {}
