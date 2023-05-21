#pragma once
// Common definitions for BTS7XXX-series load switches.
// Most of the functions in this file allow for a common API to easily interact with the
// EN pin(s) on the load switches, abstracting the specific method of accessing the pin.

#include "adc.h"
#include "gpio.h"
#include "pca9555_gpio_expander.h"
#include "status.h"

// Represents whether SEL/EN pins are accessed through STM32 or a Pca9555
typedef enum {
  BTS7XXX_PIN_STM32 = 0,
  BTS7XXX_PIN_PCA9555,
  NUM_BTS7XXX_PIN_TYPES,
} Bts7xxxPinType;

// Holds pin-specific info for EN pins
typedef struct {
  GpioAddress *enable_pin_stm32;
  Pca9555GpioAddress *enable_pin_pca9555;
  Bts7xxxPinType pin_type;
} Bts7xxxEnablePin;

// Holds pin-specific info for SEL pins
typedef struct {
  GpioAddress *select_pin_stm32;
  Pca9555GpioAddress *select_pin_pca9555;
  Bts7xxxPinType pin_type;
} Bts7xxxSelectPin;

// Holds info for the select pin state
typedef struct {
  GpioState *select_state_stm32;
  Pca9555GpioState *select_state_pca9555;
} Bts7xxxSelectPinState;

// Holds info for a BTS load switch
typedef struct {
  Bts7xxxEnablePin *enable_pin;
  Bts7xxxSelectPin *select_pin;
  GpioAddress *sense_pin;
  Bts7xxxSelectPinState select_state;
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
  uint16_t reading_out;           // Reading from IN pin, in mA
} BtsLoadSwitch;

// Broad function to initialize the pin passed in.
// If using with PCA9555, pca9555_gpio_init must be called on the i2c port and address of this pin
// prior to calling this function.
StatusCode bts_switch_init_pin(Bts7xxxEnablePin *pin);

// Broad function to enable the pin passed in.
StatusCode bts_switch_enable_pin(Bts7xxxEnablePin *pin);

// Broad function to disable the pin passed in.
StatusCode bts_switch_disable_pin(Bts7xxxEnablePin *pin);

// Broad function to get whether the pin passed in is enabled.
StatusCode bts_switch_get_pin_enabled(Bts7xxxEnablePin *pin);

//////////////////////////////////////////////////////////////////////

// Initialize the BTS load switch.
StatusCode bts_switch_init(BtsLoadSwitch *loadSwitch);

// Enable output by pulling the IN pin high.
StatusCode bts_switch_enable_output(BtsLoadSwitch *loadSwitch);

// Disable output by pulling the IN pin low.
StatusCode bts_switch_disable_output(BtsLoadSwitch *loadSwitch);

// Returns whether the output is enabled or disabled.
StatusCode bts_switch_get_output_enabled(BtsLoadSwitch *loadSwitch);

// Read the latest current input current measurement, in mA.
StatusCode bts_switch_get_current(BtsLoadSwitch *loadSwitch, uint16_t *current);

// gpio and pca init must be called before using load switch
// GPIO_OUTPUT_OPEN_DRAIN ?? -> init_pin
// Should pin_enabled specicially return a StatusCode for both cases
// what does setting the state do in regards to select pin
// scons run properly
