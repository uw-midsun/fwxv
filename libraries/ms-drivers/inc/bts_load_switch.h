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

// Holds pin-specific info for EN/SEL pins
typedef struct {
  GpioAddress *pin_stm32;
  Pca9555GpioAddress *pin_pca9555;
  Bts7xxxPinType pin_type;
} Bts7xxxPin;

// Holds info for the select pin state
typedef struct {
  GpioState select_state_stm32;
  Pca9555GpioState select_state_pca9555;
} Bts7xxxSelectPinState;

// Holds info for a BTS load switch output
// Seperate outputs on the same switch must be initialized seperately
typedef struct {
  Bts7xxxPin *enable_pin;
  Bts7xxxPin *select_pin;
  GpioAddress *sense_pin;
  Bts7xxxSelectPinState select_state;
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
  uint16_t reading_out;           // Reading from IN pin, in mA
} BtsLoadSwitchOutput;

// Initialize the BTS load switch.
StatusCode bts_output_init(BtsLoadSwitchOutput *loadSwitch);

// Enable output by pulling the IN pin high.
StatusCode bts_output_enable_output(BtsLoadSwitchOutput *loadSwitch);

// Disable output by pulling the IN pin low.
StatusCode bts_output_disable_output(BtsLoadSwitchOutput *loadSwitch);

// Returns whether the output is enabled or disabled.
StatusCode bts_output_get_enabled(BtsLoadSwitchOutput *loadSwitch);

// Read the latest current input current measurement, in mA.
StatusCode bts_output_get_current(BtsLoadSwitchOutput *loadSwitch, uint16_t *current);
