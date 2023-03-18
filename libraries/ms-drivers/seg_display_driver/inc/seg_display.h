#pragma once
#include "gpio.h"

// Functions to initialize and then set integers and floats to the 7 segment display
typedef struct SegDisplay {
  const GpioAddress A;
  const GpioAddress B;
  const GpioAddress C;
  const GpioAddress D;
  const GpioAddress DP;
  const GpioAddress D1;
  const GpioAddress D2;
  const GpioAddress D3;
} SegDisplay;

// Initializes input and display GPIOs
StatusCode seg_display_init(SegDisplay *display);

// Sets an integer value onto the display with a max value of 999
StatusCode seg_display_set_int(SegDisplay *display, uint16_t val);

// Sets a single digit decimal value onto the display with a max value of 99.9
StatusCode set_display_set_float(SegDisplay *display, float val);
