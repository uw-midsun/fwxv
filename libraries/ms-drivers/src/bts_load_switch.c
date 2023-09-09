#include "bts_load_switch.h"

#include "gpio.h"

// Broad function to initialize the pin passed in.
// If using with PCA9555, pca9555_gpio_init must be called on the i2c port and address of this pin
// prior to calling this function.
static StatusCode prv_bts_switch_init_pin(Bts7xxxPin *pin) {
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
static StatusCode prv_bts_switch_enable_pin(Bts7xxxPin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->pin_stm32, GPIO_STATE_HIGH);
  } else {
    return pca9555_gpio_set_state(pin->pin_pca9555, PCA9555_GPIO_STATE_HIGH);
  }
}

// Broad function to disable the pin passed in.
static StatusCode prv_bts_switch_disable_pin(Bts7xxxPin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->pin_stm32, GPIO_STATE_LOW);
  } else {
    return pca9555_gpio_set_state(pin->pin_pca9555, PCA9555_GPIO_STATE_LOW);
  }
}

// Broad function to get whether the pin passed in is enabled.
static StatusCode prv_bts_switch_get_pin_enabled(Bts7xxxPin *pin) {
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

static StatusCode prv_bts_switch_select_state(BtsLoadSwitchOutput *load_switch) {
  switch (load_switch->select_pin->pin_type) {
    case BTS7XXX_PIN_STM32:
      return gpio_set_state(load_switch->select_pin->pin_stm32,
                            load_switch->select_state.select_state_stm32);
    case BTS7XXX_PIN_PCA9555:
      return pca9555_gpio_set_state(load_switch->select_pin->pin_pca9555,
                                    load_switch->select_state.select_state_pca9555);
    default:
      break;
  }
  return STATUS_CODE_OK;
}

StatusCode bts_output_init(BtsLoadSwitchOutput *load_switch) {
  status_ok_or_return(gpio_init_pin(load_switch->sense_pin, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(prv_bts_switch_init_pin(load_switch->select_pin));
  return prv_bts_switch_init_pin(load_switch->enable_pin);
}

StatusCode bts_output_enable_output(BtsLoadSwitchOutput *load_switch) {
  return prv_bts_switch_enable_pin(load_switch->enable_pin);
}

StatusCode bts_output_disable_output(BtsLoadSwitchOutput *load_switch) {
  return prv_bts_switch_disable_pin(load_switch->enable_pin);
}

StatusCode bts_output_get_output_enabled(BtsLoadSwitchOutput *load_switch) {
  return prv_bts_switch_get_pin_enabled(load_switch->enable_pin);
}

StatusCode bts_output_get_current(BtsLoadSwitchOutput *load_switch, uint16_t *current) {
  status_ok_or_return(prv_bts_switch_select_state(load_switch));
  return adc_read_converted(*(load_switch->sense_pin), current);
}
