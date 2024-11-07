#pragma once
#include "gpio.h"

// Functions interact with three 7 segment displays
// Initializes all the gpios necessary for three 7 segment displays operation and sets both integers
// and floats to the displays

typedef struct SegDisplay {
  const GpioAddress A1;
  const GpioAddress B1;
  const GpioAddress C1;
  const GpioAddress D1;
  const GpioAddress A2;
  const GpioAddress B2;
  const GpioAddress C2;
  const GpioAddress D2;
  const GpioAddress A3;
  const GpioAddress B3;
  const GpioAddress C3;
  const GpioAddress D3;
  const GpioAddress DP;
  const GpioAddress Digit1;
  const GpioAddress Digit2;
  const GpioAddress Digit3;
} SegDisplay;

// Initializes input and display GPIOs
StatusCode seg_displays_init(SegDisplay *display);

// Sets an integer value onto the display with a max value of 999
StatusCode seg_displays_set_int(SegDisplay *display, uint16_t val1, uint16_t val2, uint16_t val3);

// Sets a decimal value onto the display for val1, and integer value for val2 and val3
StatusCode seg_displays_set_float(SegDisplay *display, float val1, uint16_t val2, uint16_t val3);
