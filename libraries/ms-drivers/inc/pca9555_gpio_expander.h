#pragma once
// GPIO HAL interface for the PCA9555 GPIO expander.
// Requires I2C to be initialized.
// If you want to subscribe to interrupts, you must initialize GPIO interrupts.
// Note: we don't check the validity of the I2C address, and it can only be used on one
// I2C port on one board.
#include "gpio_it.h"
#include "i2c.h"

// Addresses of the 16 pins.
typedef enum {
  PCA9555_PIN_IO0_0 = 0,
  PCA9555_PIN_IO0_1,
  PCA9555_PIN_IO0_2,
  PCA9555_PIN_IO0_3,
  PCA9555_PIN_IO0_4,
  PCA9555_PIN_IO0_5,
  PCA9555_PIN_IO0_6,
  PCA9555_PIN_IO0_7,
  PCA9555_PIN_IO1_0,
  PCA9555_PIN_IO1_1,
  PCA9555_PIN_IO1_2,
  PCA9555_PIN_IO1_3,
  PCA9555_PIN_IO1_4,
  PCA9555_PIN_IO1_5,
  PCA9555_PIN_IO1_6,
  PCA9555_PIN_IO1_7,
  NUM_PCA9555_GPIO_PINS,
} Pca9555PinAddress;

// GPIO address used to access the pin.
typedef struct {
  I2CAddress i2c_address;
  Pca9555PinAddress pin;
} Pca9555GpioAddress;

// For setting the direction of the pin.
typedef enum {
  PCA9555_GPIO_DIR_IN = 0,
  PCA9555_GPIO_DIR_OUT,
  NUM_PCA9555_GPIO_DIRS,
} Pca9555GpioDirection;

// For setting the output value of the pin.
typedef enum {
  PCA9555_GPIO_STATE_LOW = 0,
  PCA9555_GPIO_STATE_HIGH,
  NUM_PCA9555_GPIO_STATES,
} Pca9555GpioState;

typedef struct {
  Pca9555GpioDirection direction;
  Pca9555GpioState state;
} Pca9555GpioSettings;

// Initialize PCA9555 GPIO at this I2C port and address.
StatusCode pca9555_gpio_init(const I2CPort i2c_port);

// Initialize an PCA9555 GPIO pin by address.
StatusCode pca9555_gpio_init_pin(const Pca9555GpioAddress *address,
                                 const Pca9555GpioSettings *settings);

// Set the state of an PCA9555 GPIO pin by address.
StatusCode pca9555_gpio_set_state(const Pca9555GpioAddress *address, const Pca9555GpioState state);

// Toggle the output state of the pin.
StatusCode pca9555_gpio_toggle_state(const Pca9555GpioAddress *address);

// Get the value of the input register for a pin.
StatusCode pca9555_gpio_get_state(const Pca9555GpioAddress *address, Pca9555GpioState *input_state);

// Sets up a gpio interrupt on the given pin and calls the callback when it goes low

// Callback MUST CALL pca9555_gpio_get_state on the (possible) toggled pin(s),
// as the interrupt is reset when the data is read from the port that generated the interrupt
StatusCode pca9555_gpio_subscribe_interrupts(const GpioAddress *interrupt_pin, Event event,
                                             Task *task, void *context);

// Reads both of the PCA9555 8-bit registers, and returns a 16-bit value with all the pin stats.
StatusCode pca9555_get_register_state(I2CAddress addr, uint16_t *reg_val);
