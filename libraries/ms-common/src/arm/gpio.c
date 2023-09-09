#include "gpio.h"

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "status.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "task.h"

static int s_gpio_mode_map[] = {
  [GPIO_ANALOG] = GPIO_Mode_AIN,
  [GPIO_INPUT_FLOATING] = GPIO_Mode_IN_FLOATING,
  [GPIO_INPUT_PULL_DOWN] = GPIO_Mode_IPD,
  [GPIO_INPUT_PULL_UP] = GPIO_Mode_IPU,
  [GPIO_OUTPUT_OPEN_DRAIN] = GPIO_Mode_Out_OD,
  [GPIO_OUTPUT_PUSH_PULL] = GPIO_Mode_Out_PP,
  [GPIO_ALFTN_OPEN_DRAIN] = GPIO_Mode_AF_OD,
  [GPIO_ALTFN_PUSH_PULL] = GPIO_Mode_AF_PP,
};

static GPIO_TypeDef *s_gpio_port_map[NUM_GPIO_PORTS] = { GPIOA, GPIOB, GPIOC, GPIOD,
                                                         GPIOE, GPIOF, GPIOG };
static uint32_t s_gpio_rcc_apb_timer_map[NUM_GPIO_PORTS] = {
  RCC_APB2Periph_GPIOA, RCC_APB2Periph_GPIOB, RCC_APB2Periph_GPIOC, RCC_APB2Periph_GPIOD,
  RCC_APB2Periph_GPIOE, RCC_APB2Periph_GPIOF, RCC_APB2Periph_GPIOF
};

StatusCode gpio_init(void) {
  // Remap pins to disable jtag and enable LEDs
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode,
                         GpioState init_state) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      pin_mode >= NUM_GPIO_MODES) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t pin = 0x01 << address->pin;
  RCC_APB2PeriphClockCmd(s_gpio_rcc_apb_timer_map[address->port], ENABLE);

  // Create init struct for stdperiph call to initialize pin
  GPIO_InitTypeDef init_struct = {
    .GPIO_Pin = pin,
    .GPIO_Speed = GPIO_Speed_50MHz,  // Default to fastes speed
    .GPIO_Mode = s_gpio_mode_map[pin_mode],
  };

  // Use the init_struct to set the pin.
  GPIO_Init(s_gpio_port_map[address->port], &init_struct);

  // Set the pin state.
  gpio_set_state(address, init_state);

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

  GPIO_WriteBit(s_gpio_port_map[address->port], 0x01 << address->pin, (BitAction)state);

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(const GpioAddress *address) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t pin = 0x01 << address->pin;
  uint8_t state = GPIO_ReadOutputDataBit(s_gpio_port_map[address->port], pin);
  if (state) {
    GPIO_ResetBits(s_gpio_port_map[address->port], pin);
  } else {
    GPIO_SetBits(s_gpio_port_map[address->port], pin);
  }

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_get_state(const GpioAddress *address, GpioState *input_state) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *input_state = GPIO_ReadInputDataBit(s_gpio_port_map[address->port], 0x01 << address->pin);

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}
