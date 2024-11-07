#include "gpio.h"

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "log.h"
#include "status.h"
#include "task.h"

static GpioMode s_gpio_pin_modes[GPIO_TOTAL_PINS];
static uint8_t s_gpio_pin_state[GPIO_TOTAL_PINS];

static uint32_t prv_get_index(const GpioAddress *address) {
  return address->port * (uint32_t)GPIO_PINS_PER_PORT + address->pin;
}

StatusCode gpio_init(void) {
  for (uint32_t i = 0; i < GPIO_TOTAL_PINS; i++) {
    s_gpio_pin_state[i] = GPIO_STATE_LOW;
  }

  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode,
                         GpioState init_state) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      pin_mode >= NUM_GPIO_MODES || init_state >= NUM_GPIO_STATES) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_gpio_pin_state[prv_get_index(address)] = init_state;
  s_gpio_pin_modes[prv_get_index(address)] = pin_mode;

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_set_state(const GpioAddress *address, GpioState state) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      state >= NUM_GPIO_STATES) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  GpioMode mode = s_gpio_pin_modes[prv_get_index(address)];
  if (mode != GPIO_OUTPUT_OPEN_DRAIN && mode != GPIO_OUTPUT_PUSH_PULL) {
    LOG_WARN("Attempting to set an input pin, check your configuration");
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_gpio_pin_state[prv_get_index(address)] = state;

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(const GpioAddress *address) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);
  if (s_gpio_pin_state[index] == GPIO_STATE_LOW) {
    s_gpio_pin_state[index] = GPIO_STATE_HIGH;
  } else {
    s_gpio_pin_state[index] = GPIO_STATE_LOW;
  }

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_get_state(const GpioAddress *address, GpioState *state) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);

  *state = s_gpio_pin_state[index];

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}
