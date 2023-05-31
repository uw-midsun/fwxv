#include "bts_load_switch.h"

#include "gpio.h"

// Broad function to initialize the pin passed in.
// If using with PCA9555, pca9555_gpio_init must be called on the i2c port and address of this pin
// prior to calling this function.
static StatusCode bts_switch_init_pin(Bts7xxxPin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_init_pin(pin->pin_stm32, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  } else {
    Pca9555GpioSettings settings = {
      .direction = PCA9555_GPIO_DIR_OUT,
      .state = PCA9555_GPIO_STATE_LOW,
    };
    return pca9555_gpio_init_pin(pin->pin_pca9555, &settings);
  }
}

// Broad function to enable the pin passed in.
static StatusCode bts_switch_enable_pin(Bts7xxxPin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->pin_stm32, GPIO_STATE_HIGH);
  } else {
    return pca9555_gpio_set_state(pin->pin_pca9555, PCA9555_GPIO_STATE_HIGH);
  }
}

// Broad function to disable the pin passed in.
static StatusCode bts_switch_disable_pin(Bts7xxxPin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->pin_stm32, GPIO_STATE_LOW);
  } else {
    return pca9555_gpio_set_state(pin->pin_pca9555, PCA9555_GPIO_STATE_LOW);
  }
}

// Broad function to get whether the pin passed in is enabled.
static StatusCode bts_switch_get_pin_enabled(Bts7xxxPin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    GpioState pin_state;
    StatusCode status = gpio_get_state(pin->pin_stm32, &pin_state);
    return (pin_state == GPIO_STATE_HIGH);
  } else {
    Pca9555GpioState pin_state;
    StatusCode status = pca9555_gpio_get_state(pin->pin_pca9555, &pin_state);
    return (pin_state == PCA9555_GPIO_STATE_HIGH);
  }
}

StatusCode bts_switch_select_state(BtsLoadSwitchOutput *loadSwitch) {
  switch (loadSwitch->select_pin->pin_type) {
    case BTS7XXX_PIN_STM32:
      return gpio_set_state(loadSwitch->select_pin->pin_stm32,
                            loadSwitch->select_state.select_state_stm32);
    case BTS7XXX_PIN_PCA9555:
      return gpio_set_state(loadSwitch->select_pin->pin_pca9555,
                            loadSwitch->select_state.select_state_pca9555);
    default:  // NUM_BTS7XXX_PIN_TYPES - single output BTS
      break;
  }
  return STATUS_CODE_OK;
}

StatusCode bts_switch_init(BtsLoadSwitchOutput *loadSwitch) {
  // sense pin
  status_ok_or_return(gpio_init_pin(loadSwitch->sense_pin, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  // select pin
  status_ok_or_return(bts_switch_init_pin(loadSwitch->select_pin));
  // enable pin
  return bts_switch_init_pin(loadSwitch->enable_pin);
}

StatusCode bts_output_enable_output(BtsLoadSwitchOutput *loadSwitch) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return bts_switch_enable_pin(loadSwitch->enable_pin);
}

StatusCode bts_output_disable_output(BtsLoadSwitchOutput *loadSwitch) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return bts_switch_disable_pin(loadSwitch->enable_pin);
}

StatusCode bts_output_get_output_enabled(BtsLoadSwitchOutput *loadSwitch) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return bts_switch_get_pin_enabled(loadSwitch->enable_pin);
}

StatusCode bts_output_get_current(BtsLoadSwitchOutput *loadSwitch, uint16_t *current) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return adc_read_converted(*(loadSwitch->sense_pin), current);
}
