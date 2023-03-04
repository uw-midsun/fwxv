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

typedef struct DispPins {
  const GpioAddress D1;
  const GpioAddress D2;
  const GpioAddress D3;
} DispPins;

StatusCode seg_display_init();

StatusCode seg_display_set_int(uint16_t val);

StatusCode set_display_set_float(float val);

void set_seg_display(uint8_t disp_value, uint8_t pin_number);

void seg_gpio_init();
