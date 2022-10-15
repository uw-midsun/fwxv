#pragma once
// GPIO HAL Interface
#include <stdint.h>

#include "gpio_mcu.h"
#include "status.h"
#include "stm32f10x_gpio.h"

// GPIO address to be used to change that pin's settings. Both port and pin are
// zero indexed.
typedef struct GpioAddress {
  uint8_t port;
  uint8_t pin;
} GpioAddress;

// For setting the output value of the pin
typedef enum {
  GPIO_STATE_LOW = 0,
  GPIO_STATE_HIGH,
  NUM_GPIO_STATES,
} GpioState;

// Available modes for the GPIO pins
// See Section 9.1.11 of stm32f10x reference manual for
// configurations needed for different peripherals
typedef enum {
  GPIO_ANALOG = GPIO_Mode_AIN,
  GPIO_INPUT_FLOATING = GPIO_Mode_IN_FLOATING,
  GPIO_INPUT_PULL_DOWN = GPIO_Mode_IPD,
  GPIO_INPUT_PULL_UP = GPIO_Mode_IPU,
  GPIO_OUTPUT_OPEN_DRAIN = GPIO_Mode_Out_OD,
  GPIO_OUTPUT_PUSH_PULL = GPIO_Mode_Out_PP,
  GPIO_ALFTN_OPEN_DRAIN = GPIO_Mode_AF_OD,
  GPIO_ALTFN_PUSH_PULL = GPIO_Mode_AF_PP,
  NUM_GPIO_MODES,
} GpioMode;

// Initializes GPIO globally by setting all pins to their default state. ONLY
// CALL ONCE or it will deinit all current settings. Change setting by calling
// gpio_init_pin.
StatusCode gpio_init(void);

// Initializes a GPIO pin by address.
// GPIOs are configured to a specified mode, at the max refresh speed
// The init_state only matters if the pin is configured as an output
StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode, GpioState init_state);

// Set the pin state by address.
StatusCode gpio_set_state(const GpioAddress *address, GpioState state);

// Toggles the output state of the pin.
StatusCode gpio_toggle_state(const GpioAddress *address);

// Gets the value of the input register for a pin and assigns it to the state
// that is passed in.
StatusCode gpio_get_state(const GpioAddress *address, GpioState *input_state);
