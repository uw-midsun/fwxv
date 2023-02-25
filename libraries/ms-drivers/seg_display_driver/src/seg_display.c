#include "seg_display.h"

#include "bcd.h"
#include "log.h"

StatusCode seg_display_init(PinsBcd *bcd_pins, DispPins *disp_sel_pins) {
  seg_gpio_init(bcd_pins, disp_sel_pins);

  set_seg_display(8, 0, bcd_pins, disp_sel_pins);
  set_seg_display(8, 1, bcd_pins, disp_sel_pins);
  set_seg_display(8, 2, bcd_pins, disp_sel_pins);
  return STATUS_CODE_OK;
}

StatusCode seg_display_set_int(uint16_t val, PinsBcd *bcd_pins, DispPins *disp_sel_pins) {
  // Check for max value
  if (val > 999) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    set_seg_display(val % 10, 0, bcd_pins, disp_sel_pins);
    val = val / 10;
    set_seg_display(val % 10, 1, bcd_pins, disp_sel_pins);
    val = val / 10;
    set_seg_display(val % 10, 2, bcd_pins, disp_sel_pins);
    return STATUS_CODE_OK;
  }
}

StatusCode set_display_set_float(float val, PinsBcd *bcd_pins, DispPins *disp_sel_pins) {
  int int_part = (int)val;
  int dec_part = (int)((val - (float)int_part) * 10);
  if (int_part > 99) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    set_seg_display(int_part % 10, 1, bcd_pins, disp_sel_pins);
    int_part = int_part / 10;
    set_seg_display(int_part % 10, 2, bcd_pins, disp_sel_pins);
    set_seg_display(dec_part, 0, bcd_pins, disp_sel_pins);
    // Set decimal point
    gpio_set_state(&bcd_pins->DP, GPIO_STATE_HIGH);
    gpio_toggle_state(&disp_sel_pins->D2);
    gpio_toggle_state(&disp_sel_pins->D2);
    return STATUS_CODE_OK;
  }
}

void seg_gpio_init(PinsBcd *bcd_pins, DispPins *disp_sel_pins) {
  // Initializes gpio pins
  gpio_init_pin(&bcd_pins->A, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&bcd_pins->B, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&bcd_pins->C, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&bcd_pins->D, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&bcd_pins->DP, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&disp_sel_pins->D1, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&disp_sel_pins->D2, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&disp_sel_pins->D3, GPIO_ANALOG, GPIO_STATE_LOW);
}

void set_seg_display(uint8_t disp_value, uint8_t pin_number, PinsBcd *bcd_pins,
                     DispPins *disp_sel_pins) {
  // Sets A-D with the binary digits of 8
  uint8_t A = 1 ? (dec_to_bcd(disp_value) | 1) == dec_to_bcd(disp_value) : 0;
  uint8_t B = 1 ? (dec_to_bcd(disp_value) | 2) == dec_to_bcd(disp_value) : 0;
  uint8_t C = 1 ? (dec_to_bcd(disp_value) | 4) == dec_to_bcd(disp_value) : 0;
  uint8_t D = 1 ? (dec_to_bcd(disp_value) | 8) == dec_to_bcd(disp_value) : 0;
  // Uses values of A, B, C, D to set bcd_pins' states
  if (A)
    gpio_set_state(&bcd_pins->A, GPIO_STATE_HIGH);
  else
    gpio_set_state(&bcd_pins->A, GPIO_STATE_LOW);
  if (B)
    gpio_set_state(&bcd_pins->B, GPIO_STATE_HIGH);
  else
    gpio_set_state(&bcd_pins->B, GPIO_STATE_LOW);
  if (C)
    gpio_set_state(&bcd_pins->C, GPIO_STATE_HIGH);
  else
    gpio_set_state(&bcd_pins->C, GPIO_STATE_LOW);
  if (D)
    gpio_set_state(&bcd_pins->D, GPIO_STATE_HIGH);
  else
    gpio_set_state(&bcd_pins->D, GPIO_STATE_LOW);

  // Toggles the disp_sel_pin specified twice to set the value in the display
  if (pin_number == 0) {
    gpio_toggle_state(&disp_sel_pins->D1);
    gpio_toggle_state(&disp_sel_pins->D1);
  }
  if (pin_number == 1) {
    gpio_toggle_state(&disp_sel_pins->D2);
    gpio_toggle_state(&disp_sel_pins->D2);
  }
  if (pin_number == 2) {
    gpio_toggle_state(&disp_sel_pins->D3);
    gpio_toggle_state(&disp_sel_pins->D3);
  }
}
