#include "gpio.h"

#include <stdbool.h>
#include <stdint.h>

#include "log.h"
#include "status.h"
#include "mutex.h"

static GpioSettings s_pin_settings[GPIO_TOTAL_PINS];
static uint8_t s_gpio_pin_input_value[GPIO_TOTAL_PINS];
static Mutex s_gpioMutex;

static uint32_t prv_get_index(const GpioAddress *address) {
  return address->port * (uint32_t)GPIO_PINS_PER_PORT + address->pin;
}

StatusCode gpio_init(void) {
  GpioSettings default_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  for (uint32_t i = 0; i < GPIO_TOTAL_PINS; i++) {
    s_pin_settings[i] = default_settings;
    s_gpio_pin_input_value[i] = 0;
  }
  StatusCode status = mutex_init(&s_gpioMutex);

  if (status != STATUS_CODE_OK) {
    return status_code(status)
  }

  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioSettings *settings) {
  StatusCode status = mutex_lock(&s_gpioMutex, BLOCK_INDEFINITELY);

  if (status != STATUS_CODE_OK) {
    return status_code(status);
  } else if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      settings->direction >= NUM_GPIO_DIRS || settings->state >= NUM_GPIO_STATES ||
      settings->resistor >= NUM_GPIO_RESES || settings->alt_function >= NUM_GPIO_ALTFNS) {
    mutex_unlock(&s_gpioMutex);
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[prv_get_index(address)] = *settings;
  mutex_unlock(&s_gpioMutex);
  return STATUS_CODE_OK;
}

StatusCode gpio_set_state(const GpioAddress *address, GpioState state) {
  StatusCode status = mutex_lock(&s_gpioMutex, BLOCK_INDEFINITELY);

  if (status != STATUS_CODE_OK) {
    return status_code(status);
  } else if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
    state >= NUM_GPIO_STATES) {
    mutex_unlock(&s_gpioMutex);
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[prv_get_index(address)].state = state;
  mutex_unlock(&s_gpioMutex);
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(const GpioAddress *address) {
  StatusCode status = mutex_lock(&s_gpioMutex, BLOCK_INDEFINITELY);

  if (status != STATUS_CODE_OK) {
    return status_code(status);
  } else if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    mutex_unlock(&s_gpioMutex);
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);
  if (s_pin_settings[index].state == GPIO_STATE_LOW) {
    s_pin_settings[index].state = GPIO_STATE_HIGH;
  } else {
    s_pin_settings[index].state = GPIO_STATE_LOW;
  }

  mutex_unlock(&s_gpioMutex);
  return STATUS_CODE_OK;
}

StatusCode gpio_get_state(const GpioAddress *address, GpioState *state) {
  StatusCode status = mutex_lock(&s_gpioMutex, BLOCK_INDEFINITELY);

  if (status != STATUS_CODE_OK) {
    return status_code(status);
  } else if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    mutex_unlock(&s_gpioMutex);
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);

  // Behave how hardware does when the direction is set to out.
  if (s_pin_settings[index].direction != GPIO_DIR_IN) {
    *state = s_pin_settings[index].state;
  } else {
    *state = s_gpio_pin_input_value[index];
  }

  mutex_unlock(&s_gpioMutex);
  return STATUS_CODE_OK;
}
