#include "seg_display.h"

#include "log.h"

StatusCode seg_display_init(SegDisplay *display) {
  gpio_init_pin(&display->A, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->B, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->C, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->DP, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  set_seg_display(display, 8, 0);
  set_seg_display(display, 8, 1);
  set_seg_display(display, 8, 2);
  return STATUS_CODE_OK;
}

StatusCode seg_display_set_int(SegDisplay *display, uint16_t val) {
  // Check for max value
  if (val > 999) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    set_seg_display(display, val % 10, 0);
    val = val / 10;
    set_seg_display(display, val % 10, 1);
    val = val / 10;
    set_seg_display(display, val % 10, 2);
    return STATUS_CODE_OK;
  }
}

StatusCode set_display_set_float(SegDisplay *display, float val) {
  int int_part = (int)val;
  int dec_part = (int)((val - (float)int_part) * 10);
  if (int_part > 99) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    set_seg_display(display, int_part % 10, 1);
    int_part = int_part / 10;
    set_seg_display(display, int_part % 10, 2);
    set_seg_display(display, dec_part, 0);
    // Set decimal point
    gpio_set_state(&display->DP, GPIO_STATE_HIGH);
    return STATUS_CODE_OK;
  }
}

void set_seg_display(SegDisplay *display, uint8_t disp_value, uint8_t pin_number);
void set_seg_display(SegDisplay *display, uint8_t disp_value, uint8_t pin_number) {
  // Sets A-D with the binary digits of 8
  GpioState state_a = disp_value & 0x1;
  GpioState state_b = disp_value & 0x2;
  GpioState state_c = disp_value & 0x4;
  GpioState state_d = disp_value & 0x8;

  // Uses values of A, B, C, D to set bcd_pins' states
  gpio_set_state(&display->A, state_a);
  gpio_set_state(&display->B, state_b);
  gpio_set_state(&display->C, state_c);
  gpio_set_state(&display->D, state_d);

  // Toggles the disp_sel_pin specified twice to set the value in the display
  if (pin_number == 0) {
    gpio_toggle_state(&display->D1);
    gpio_toggle_state(&display->D1);
  }
  if (pin_number == 1) {
    gpio_toggle_state(&display->D2);
    gpio_toggle_state(&display->D2);
  }
  if (pin_number == 2) {
    gpio_toggle_state(&display->D3);
    gpio_toggle_state(&display->D3);
  }
}
