#include "pca9555_gpio_expander.h"

#include <stdbool.h>

#include "pca9555_gpio_expander_defs.h"

// The I2C port used for all operations - won't change on each board
static I2CPort s_i2c_port = NUM_I2C_PORTS;

static bool prv_is_port_0(const Pca9555PinAddress pin) {
  return pin < PCA9555_PIN_IO1_0;
}

static uint8_t prv_select_reg(const Pca9555PinAddress pin, uint8_t reg0, uint8_t reg1) {
  return prv_is_port_0(pin) ? reg0 : reg1;
}

// get the bit representing this pin in an 8-bit register
static uint8_t prv_pin_bit(const Pca9555PinAddress pin) {
  return prv_is_port_0(pin) ? pin : pin - PCA9555_PIN_IO1_0;
}

StatusCode pca9555_gpio_init(const I2CPort i2c_port) {
  if (s_i2c_port == NUM_I2C_PORTS) {
    s_i2c_port = i2c_port;
  }
  return STATUS_CODE_OK;
}

static void prv_read_reg(const Pca9555GpioAddress *address, uint8_t reg0, uint8_t reg1,
                         uint8_t *rx_data) {
  uint8_t reg = prv_select_reg(address->pin, reg0, reg1);
  i2c_read_reg(s_i2c_port, address->i2c_address, reg, rx_data, 1);
}

static void prv_write_reg(const Pca9555GpioAddress *address, uint8_t reg0, uint8_t reg1,
                          uint8_t tx_data) {
  // PCA9555 expects the register ("command byte") as just a data byte (see figs 10, 11)
  uint8_t reg = prv_select_reg(address->pin, reg0, reg1);
  uint8_t data[] = { reg, tx_data };
  i2c_write(s_i2c_port, address->i2c_address, data, SIZEOF_ARRAY(data));
}

static void prv_set_reg_bit(const Pca9555GpioAddress *address, uint8_t reg0, uint8_t reg1,
                            bool val) {
  uint8_t bit = prv_pin_bit(address->pin);
  uint8_t data = 0;
  prv_read_reg(address, reg0, reg1, &data);
  if (val) {
    data |= 1 << bit;
  } else {
    data &= ~(1 << bit);
  }
  prv_write_reg(address, reg0, reg1, data);
}

StatusCode pca9555_gpio_init_pin(const Pca9555GpioAddress *address,
                                 const Pca9555GpioSettings *settings) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9555_GPIO_PINS || settings->direction >= NUM_PCA9555_GPIO_DIRS ||
      settings->state >= NUM_PCA9555_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the IODIR bit; 0 = output, 1 = input
  prv_set_reg_bit(address, IODIR0, IODIR1, settings->direction == PCA9555_GPIO_DIR_IN);

  if (settings->direction == PCA9555_GPIO_DIR_OUT) {
    prv_set_reg_bit(address, OUTPUT0, OUTPUT1, settings->state);
  }

  return STATUS_CODE_OK;
}

StatusCode pca9555_gpio_set_state(const Pca9555GpioAddress *address, const Pca9555GpioState state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9555_GPIO_PINS || state >= NUM_PCA9555_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_set_reg_bit(address, OUTPUT0, OUTPUT1, state == PCA9555_GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}

StatusCode pca9555_gpio_toggle_state(const Pca9555GpioAddress *address) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9555_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // optimization: instead of using set_state and get_state, we read only once
  uint8_t gpio_data = 0;
  prv_read_reg(address, OUTPUT0, OUTPUT1, &gpio_data);
  gpio_data ^= 1 << prv_pin_bit(address->pin);
  prv_write_reg(address, OUTPUT0, OUTPUT1, gpio_data);

  return STATUS_CODE_OK;
}

StatusCode pca9555_gpio_get_state(const Pca9555GpioAddress *address,
                                  Pca9555GpioState *input_state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9555_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t gpio_data = 0;
  prv_read_reg(address, INPUT0, INPUT1, &gpio_data);

  // Read the |bit|th bit
  uint8_t bit = prv_pin_bit(address->pin);
  *input_state = ((gpio_data & (1 << bit)) == 0) ? PCA9555_GPIO_STATE_LOW : PCA9555_GPIO_STATE_HIGH;
  return STATUS_CODE_OK;
}

StatusCode pca9555_gpio_subscribe_interrupts(const GpioAddress *interrupt_pin, Event event,
                                             Task *task, void *context) {
  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .edge = INTERRUPT_EDGE_FALLING  // open-drain active low
  };
  gpio_it_register_interrupt(interrupt_pin, &interrupt_settings, event, task);
  return STATUS_CODE_OK;
}
