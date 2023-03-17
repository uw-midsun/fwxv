#include <stdbool.h>
#include <unistd.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "pca9555_gpio_expander.h"
#include "semaphore.h"
#include "status.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"

#define MOCK_I2C_ADDRESS 0b01010101
#define MOCK_I2C_PIN PCA9555_PIN_IO0_4
#define MOCK_I2C_IN_PIN PCA9555_PIN_IO0_1
#define MOCK_REG 0b00100011

static I2CSettings i2c_settings = { .scl = { .port = GPIO_PORT_A, .pin = 5 },
                                    .sda = { .port = GPIO_PORT_A, .pin = 4 },
                                    .speed = I2C_SPEED_STANDARD };

static I2CPort i2c_port = I2C_PORT_1;
static Pca9555GpioAddress pca9555_address = { .i2c_address = MOCK_I2C_ADDRESS,
                                              .pin = MOCK_I2C_PIN };

void setup_test(void) {
  log_init();
  gpio_init();
  gpio_it_init();

  i2c_init(i2c_port, &i2c_settings);
  pca9555_gpio_init(i2c_port, MOCK_I2C_ADDRESS);
}

void teardown_test(void) {}

TEST_IN_TASK

void test_gpio_set_state(void) {
  StatusCode status;
  uint8_t data = MOCK_REG;

  // set reg to a mock state
  status = i2c_write(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  status = pca9555_gpio_set_state(&pca9555_address, PCA9555_GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // reading the input reg address. Reg address should be 0x01 (OUTPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x02, data);

  // reading the output reg address. Reg address should be 0x02 (OUTPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x02, data);

  // reading reg content. Reg content should have bit 4 turned on since mock pin is pin 4.
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL((MOCK_REG | 1 << MOCK_I2C_PIN), data);
}

void test_gpio_get_state(void) {
  Pca9555GpioState state = PCA9555_GPIO_STATE_LOW;
  StatusCode status;
  uint8_t data = 1 << MOCK_I2C_PIN;

  // set reg to a mock state
  status = i2c_write(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // reading reg content. Reg content should be high because bit 4 is high in mock state
  // and pca address pin is 4.
  status = pca9555_gpio_get_state(&pca9555_address, &state);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(PCA9555_GPIO_STATE_HIGH, state);

  // reading input reg address. Should match INPUT0 (0x00)
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x00, data);
}

void test_gpio_toggle_state(void) {
  StatusCode status;
  uint8_t data = 0x00;

  // set reg to a mock state of 0
  status = i2c_write(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  status = pca9555_gpio_toggle_state(&pca9555_address);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // reading the input reg address. Reg address should be 0x01 (INPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x00, data);

  // reading the output reg address. Reg address should be 0x02 (OUTPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x02, data);

  // reading reg content. Reg content should have bit 4 turned on since mock pin is pin 4.
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(1 << MOCK_I2C_PIN, data);

  // set reg content to what it was after toggling state
  status = i2c_write(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // toggle same pin again
  status = pca9555_gpio_toggle_state(&pca9555_address);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // reading the input reg address. Reg address should be 0x00 (INPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x00, data);

  // reading the output reg address. Reg address should be 0x02 (OUTPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x02, data);

  // reading reg content. Reg content should be 0 since same pin got toggled again (turns off)
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x00, data);
}

void test_gpio_init_pin() {
  const Pca9555GpioSettings pca9555_in_settings = { .direction = PCA9555_GPIO_DIR_IN,
                                                    .state = PCA9555_GPIO_STATE_LOW };

  StatusCode status;
  uint8_t data = MOCK_REG;

  // set reg to a mock state
  status = i2c_write(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  status = pca9555_gpio_init_pin(&pca9555_address, &pca9555_in_settings);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // reading the io dir reg address. Reg address should be 0x06 (IODIR0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x06, data);

  // reading the io dir reg address. Reg address should be 0x06 (IODIR0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x06, data);

  // reading reg content. Reg content should have bit 4 low for input setting on since mock pin is
  // pin 4.
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(MOCK_REG | (1 << MOCK_I2C_PIN), data);

  const Pca9555GpioSettings pca9555_out_settings = { .direction = PCA9555_GPIO_DIR_OUT,
                                                     .state = PCA9555_GPIO_STATE_LOW };
  data = MOCK_REG | (1 << MOCK_I2C_PIN);

  // set reg to a mock state where the pin being intialized is turned on (state HIGH)
  status = i2c_write(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // set output reg to a mock state
  status = i2c_write(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  status = pca9555_gpio_init_pin(&pca9555_address, &pca9555_out_settings);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  // reading the io dir reg address. Reg address should be 0x06 (IODIR0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x06, data);

  // reading the io dir reg address. Reg address should be 0x06 (IODIR0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x06, data);

  // reading reg content. Reg content should have bit 4 low for output setting on since mock pin is
  // pin 4.
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(MOCK_REG, data);

  // reading the input reg address. Reg address should be 0x00 (OUTPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x02, data);

  // reading the output reg address. Reg address should be 0x02 (OUTPUT0) for pins 0-7
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(0x02, data);

  // reading reg content. Reg content should have bit 4 turned off since default is
  // PCA9555_GPIO_STATE_LOW
  status = i2c_read(i2c_port, MOCK_I2C_ADDRESS, &data, 1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(MOCK_REG, data);
}
