#include "gpio.h"
#include "gpio_it.h"

typedef struct PinsBcd {
  const GpioAddress A = {
    .port = GPIO_PORT_A,
    .pin = 7,
  };
  const GpioAddress B = {
    .port = GPIO_PORT_B,
    .pin = 1,
  };
  const GpioAddress C = {
    .port = GPIO_PORT_C,
    .pin = 2,
  };
  const GpioAddress D = {
    .port = GPIO_PORT_D,
    .pin = 6,
  };
  const GpioAddress DP = {
    .port = GPIO_PORT_A,
    .pin = 4,
  };
} PinsBcd;

PinsBcd *pins_bcd;

typedef struct DispPins {
  const GpioAddress D1 = {
    .port = GPIO_PORT_D,
    .pin = 1,
  };
  const GpioAddress D2 = {
    .port = GPIO_PORT_D,
    .pin = 2,
  };
  const GpioAddress D3 = {
    .port = GPIO_PORT_D,
    .pin = 3,
  };
} DispPins;

DispPins *disp_pins;

StatusCode seg_display_init(PinsBcd *bcd_pins, DispPins *disp_sel_pins);

StatusCode seg_display_set_int(uint16_t val, PinsBcd *bcd_pins, DispPins *disp_sel_pins);

StatusCode set_display_set_float(float val, PinsBcd *bcd_pins, DispPins *disp_sel_pins);

void set_seg_display(uint8_t disp_value, uint8_t pin_number, PinsBcd *bcd_pins,
                     DispPins *disp_sel_pins);

void seg_gpio_init(PinsBcd *bcd_pins, DispPins *disp_sel_pins);
