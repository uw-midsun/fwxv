#include "bts_load_switch.h"

#include "gpio.h"

StatusCode bts_switch_init_pin(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_init_pin(pin->enable_pin_stm32, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  } else {
    Pca9555GpioSettings settings = {
      .direction = PCA9555_GPIO_DIR_OUT,
      .state = PCA9555_GPIO_STATE_LOW,
    };
    return pca9555_gpio_init_pin(pin->enable_pin_pca9555, &settings);
  }
}

StatusCode bts_switch_enable_pin(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_HIGH);
  } else {
    return pca9555_gpio_set_state(pin->enable_pin_pca9555, PCA9555_GPIO_STATE_HIGH);
  }
}

StatusCode bts_switch_disable_pin(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_LOW);
  } else {
    return pca9555_gpio_set_state(pin->enable_pin_pca9555, PCA9555_GPIO_STATE_LOW);
  }
}

// Should pin_enabled specicially return a StatusCode for both cases
StatusCode bts_switch_get_pin_enabled(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    GpioState pin_state;
    StatusCode status = gpio_get_state(pin->enable_pin_stm32, &pin_state);
    return (pin_state == GPIO_STATE_HIGH);
  } else {
    Pca9555GpioState pin_state;
    StatusCode status = pca9555_gpio_get_state(pin->enable_pin_pca9555, &pin_state);
    return (pin_state == PCA9555_GPIO_STATE_HIGH);
  }
}

//////////////////////////////////////////////////////////////////////////

StatusCode bts_switch_select_state(BtsLoadSwitch *loadSwitch) {
  switch (loadSwitch->select_pin->pin_type) {
    case BTS7XXX_PIN_STM32:
      return gpio_set_state(loadSwitch->select_pin->select_pin_stm32,
                            loadSwitch->select_state.select_state_stm32);
    case BTS7XXX_PIN_PCA9555:
      return gpio_set_state(loadSwitch->select_pin->select_pin_pca9555,
                            loadSwitch->select_state.select_state_pca9555);
    default:  // NUM_BTS7XXX_PIN_TYPES - single output BTS
      break;
  }
  return STATUS_CODE_OK;
}

StatusCode bts_switch_init(BtsLoadSwitch *loadSwitch) {
  // sense pin
  // select pin
  // select state
  // enable pin
  return bts_switch_init_pin(loadSwitch->enable_pin);
}

StatusCode bts_switch_enable_output(BtsLoadSwitch *loadSwitch) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return bts_switch_enable_pin(loadSwitch->enable_pin);
}

StatusCode bts_switch_disable_output(BtsLoadSwitch *loadSwitch) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return bts_switch_disable_pin(loadSwitch->enable_pin);
}

StatusCode bts_switch_get_output_enabled(BtsLoadSwitch *loadSwitch) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return bts_switch_get_pin_enabled(loadSwitch->enable_pin);
}

StatusCode bts_switch_get_current(BtsLoadSwitch *loadSwitch, uint16_t *current) {
  status_ok_or_return(bts_switch_select_state(loadSwitch));
  return adc_read_converted(*(loadSwitch->sense_pin), current);
}
