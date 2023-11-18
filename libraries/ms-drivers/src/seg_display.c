#include "seg_display.h"

#include "delay.h"

#define MAX_DISPLAY_VALUE 999
#define DELAY_BETWEEN_DIGITS_MS 3

void set_seg_display(SegDisplay *display, uint16_t val);
void seg_display_reset(SegDisplay *display);
void set_digit(SegDisplay *display, GpioAddress *digit, uint16_t val);

StatusCode seg_display_init(SegDisplay *display) {
  gpio_init_pin(&display->A, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->B, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->C, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&display->DP, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  return STATUS_CODE_OK;
}

void seg_display_reset(SegDisplay *display) {
  gpio_set_state(&display->A, GPIO_STATE_LOW);
  gpio_set_state(&display->B, GPIO_STATE_LOW);
  gpio_set_state(&display->C, GPIO_STATE_LOW);
  gpio_set_state(&display->D, GPIO_STATE_LOW);
  gpio_set_state(&display->DP, GPIO_STATE_HIGH);
  gpio_set_state(&display->D1, GPIO_STATE_HIGH);
  gpio_set_state(&display->D2, GPIO_STATE_HIGH);
  gpio_set_state(&display->D3, GPIO_STATE_HIGH);
}

void set_seg_display(SegDisplay *display, uint16_t val) {  // Sets ABCD according to value
  GpioState state;

  state = ((val & 8) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->D, state);

  state = ((val & 4) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->C, state);

  state = ((val & 2) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->B, state);

  state = ((val & 1) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->A, state);
}

void set_digit(SegDisplay *display, GpioAddress *digit, uint16_t val) {  // Sets D1 D2 D3
  gpio_set_state(digit, GPIO_STATE_LOW);
  set_seg_display(display, val);
  delay_ms(DELAY_BETWEEN_DIGITS_MS);
  gpio_set_state(digit, GPIO_STATE_HIGH);
}

StatusCode seg_display_set_int(SegDisplay *display, uint16_t val) {
  // Check for max value
  if (val > MAX_DISPLAY_VALUE) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    seg_display_reset(display);

    set_digit(display, &display->D1, (val / 100));
    set_digit(display, &display->D2, ((val / 10) % 10));
    set_digit(display, &display->D3, (val % 10));

    return STATUS_CODE_OK;
  }
}

StatusCode seg_display_set_float(SegDisplay *display, float val) {
  uint16_t val_int = val * 10;

  if (val_int > MAX_DISPLAY_VALUE) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    seg_display_reset(display);

    set_digit(display, &display->D1, (val_int / 100));

    gpio_set_state(&display->DP, GPIO_STATE_LOW);  // Enable decimal point
    set_digit(display, &display->D2, ((val_int / 10) % 10));

    gpio_set_state(&display->DP, GPIO_STATE_HIGH);
    set_digit(display, &display->D3, (val_int % 10));

    return STATUS_CODE_OK;
  }
}
