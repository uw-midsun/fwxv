#pragma once
#include "gpio.h"
#include "gpio_it.h"
typedef struct PinsBcd {
  const GpioAddress A;
  const GpioAddress B;
  const GpioAddress C;
  const GpioAddress D;
  const GpioAddress DP;
} PinsBcd;

static const GpioAddress pins_bcd[] = {
  {
      .port = GPIO_PORT_A,
      .pin = 7,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 1,
  },
  {
      .port = GPIO_PORT_C,
      .pin = 2,
  },
  {
      .port = GPIO_PORT_D,
      .pin = 6,
  },
  {
      .port = GPIO_PORT_E,
      .pin = 1,
  },
};

typedef struct DispPins {
  const GpioAddress D1;
  const GpioAddress D2;
  const GpioAddress D3;
} DispPins;

static const GpioAddress disp_pins[] = {
  {
      .port = GPIO_PORT_D,
      .pin = 1,
  },
  {
      .port = GPIO_PORT_D,
      .pin = 2,
  },
  {
      .port = GPIO_PORT_D,
      .pin = 3,
  },
  {
      .port = GPIO_PORT_D,
      .pin = 6,
  },
};

StatusCode seg_display_init(PinsBcd *bcd_pins, DispPins *disp_sel_pins);

StatusCode seg_display_set_int(uint16_t val, PinsBcd *bcd_pins, DispPins *disp_sel_pins);

StatusCode set_display_set_float(float val, PinsBcd *bcd_pins, DispPins *disp_sel_pins);

void set_seg_display(uint8_t disp_value, uint8_t pin_number, PinsBcd *bcd_pins,
                     DispPins *disp_sel_pins);

void seg_gpio_init(PinsBcd *bcd_pins, DispPins *disp_sel_pins);
