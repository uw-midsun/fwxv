#include "seg_display.h"

#include "bcd.h"
#include "log.h"

static struct PinsBcd s_bcd_pins = {
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
static struct DispPins s_disp_sel_pins = {
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

StatusCode seg_display_init() {
  seg_gpio_init(s_bcd_pins, &s_disp_sel_pins);

  set_seg_display(8, 0);
  set_seg_display(8, 1);
  set_seg_display(8, 2);
  return STATUS_CODE_OK;
}

StatusCode seg_display_set_int(uint16_t val) {
  // Check for max value
  if (val > 999) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    set_seg_display(val % 10, 0);
    val = val / 10;
    set_seg_display(val % 10, 1);
    val = val / 10;
    set_seg_display(val % 10, 2);
    return STATUS_CODE_OK;
  }
}

StatusCode set_display_set_float(float val) {
  int int_part = (int)val;
  int dec_part = (int)((val - (float)int_part) * 10);
  if (int_part > 99) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    set_seg_display(int_part % 10, 1);
    int_part = int_part / 10;
    set_seg_display(int_part % 10, 2);
    set_seg_display(dec_part, 0);
    // Set decimal point
    gpio_set_state(&s_bcd_pins.DP, GPIO_STATE_HIGH);
    gpio_toggle_state(&s_disp_sel_pins.D2);
    gpio_toggle_state(&s_disp_sel_pins.D2);
    return STATUS_CODE_OK;
  }
}

void seg_gpio_init() {
  // Initializes gpio pins
  gpio_init_pin(&s_bcd_pins.A, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&s_bcd_pins.B, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&s_bcd_pins.C, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&s_bcd_pins.D, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&s_bcd_pins.DP, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&s_disp_sel_pins.D1, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&s_disp_sel_pins.D2, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&s_disp_sel_pins.D3, GPIO_ANALOG, GPIO_STATE_LOW);
}

void set_seg_display(uint8_t disp_value, uint8_t pin_number) {
  // Sets A-D with the binary digits of 8
  uint8_t A = 1 ? (dec_to_bcd(disp_value) | 1) == dec_to_bcd(disp_value) : 0;
  uint8_t B = 1 ? (dec_to_bcd(disp_value) | 2) == dec_to_bcd(disp_value) : 0;
  uint8_t C = 1 ? (dec_to_bcd(disp_value) | 4) == dec_to_bcd(disp_value) : 0;
  uint8_t D = 1 ? (dec_to_bcd(disp_value) | 8) == dec_to_bcd(disp_value) : 0;
  // Uses values of A, B, C, D to set bcd_pins' states
  if (A) {
    gpio_set_state(&s_bcd_pins.A, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&s_bcd_pins.A, GPIO_STATE_LOW);
  }
  if (B) {
    gpio_set_state(&s_bcd_pins.B, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&s_bcd_pins.B, GPIO_STATE_LOW);
  }
  if (C) {
    gpio_set_state(&s_bcd_pins.C, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&s_bcd_pins.C, GPIO_STATE_LOW);
  }
  if (D) {
    gpio_set_state(&s_bcd_pins.D, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&s_bcd_pins.D, GPIO_STATE_LOW);
  }
  // Toggles the disp_sel_pin specified twice to set the value in the display
  if (pin_number == 0) {
    gpio_toggle_state(&s_disp_sel_pins.D1);
    gpio_toggle_state(&s_disp_sel_pins.D1);
  }
  if (pin_number == 1) {
    gpio_toggle_state(&s_disp_sel_pins.D2);
    gpio_toggle_state(&s_disp_sel_pins.D2);
  }
  if (pin_number == 2) {
    gpio_toggle_state(&s_disp_sel_pins.D3);
    gpio_toggle_state(&s_disp_sel_pins.D3);
  }
}
