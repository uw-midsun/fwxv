#include "seg_display.h"

#include "delay.h"

#define MAX_DISPLAY_VALUE 999
#define DELAY_BETWEEN_DIGITS_MS 3

void seg_displays_reset(SegDisplay *display);
void set_digit(SegDisplay *display, GpioAddress *digit, uint16_t val1, uint16_t val2,
               uint16_t val3);

StatusCode seg_displays_init(SegDisplay *display) {
  gpio_init_pin(&display->A1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->B1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->C1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&display->A2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->B2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->C2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&display->A3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->B3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->C3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->D3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&display->DP, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->Digit1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->Digit2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&display->Digit3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  return STATUS_CODE_OK;
}

void seg_displays_reset(SegDisplay *display) {
  gpio_set_state(&display->A1, GPIO_STATE_LOW);
  gpio_set_state(&display->B1, GPIO_STATE_LOW);
  gpio_set_state(&display->C1, GPIO_STATE_LOW);
  gpio_set_state(&display->D1, GPIO_STATE_LOW);
  gpio_set_state(&display->A2, GPIO_STATE_LOW);
  gpio_set_state(&display->B2, GPIO_STATE_LOW);
  gpio_set_state(&display->C2, GPIO_STATE_LOW);
  gpio_set_state(&display->D2, GPIO_STATE_LOW);
  gpio_set_state(&display->A3, GPIO_STATE_LOW);
  gpio_set_state(&display->B3, GPIO_STATE_LOW);
  gpio_set_state(&display->C3, GPIO_STATE_LOW);
  gpio_set_state(&display->D3, GPIO_STATE_LOW);
  gpio_set_state(&display->DP, GPIO_STATE_HIGH);
  gpio_set_state(&display->Digit1, GPIO_STATE_HIGH);
  gpio_set_state(&display->Digit2, GPIO_STATE_HIGH);
  gpio_set_state(&display->Digit3, GPIO_STATE_HIGH);
}

static void prv_set_seg_displays(SegDisplay *display, uint16_t val1, uint16_t val2,
                      uint16_t val3) {  // Sets ABCD according to value
  GpioState state;

  state = ((val1 & 8) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->D1, state);

  state = ((val1 & 4) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->C1, state);

  state = ((val1 & 2) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->B1, state);

  state = ((val1 & 1) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->A1, state);

  state = ((val2 & 8) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->D2, state);

  state = ((val2 & 4) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->C2, state);

  state = ((val2 & 2) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->B2, state);

  state = ((val2 & 1) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->A2, state);

  state = ((val3 & 8) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->D3, state);

  state = ((val3 & 4) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->C3, state);

  state = ((val3 & 2) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->B3, state);

  state = ((val3 & 1) != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_set_state(&display->A3, state);
}

void set_digit(SegDisplay *display, GpioAddress *digit, uint16_t val1, uint16_t val2,
               uint16_t val3) {  // Sets D1 D2 D3
  gpio_set_state(digit, GPIO_STATE_LOW);
  prv_set_seg_displays(display, val1, val2, val3);
  delay_ms(DELAY_BETWEEN_DIGITS_MS);
  gpio_set_state(digit, GPIO_STATE_HIGH);
}

StatusCode seg_displays_set_int(SegDisplay *display, uint16_t val1, uint16_t val2, uint16_t val3) {
  // Check for max value
  if ((val1 > MAX_DISPLAY_VALUE) || (val2 > MAX_DISPLAY_VALUE) || (val3 > MAX_DISPLAY_VALUE)) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    seg_displays_reset(display);

    set_digit(display, &display->Digit1, (val1 / 100), (val2 / 100), (val3 / 100));
    set_digit(display, &display->Digit2, ((val1 / 10) % 10), ((val2 / 10) % 10),
              ((val3 / 10) % 10));
    set_digit(display, &display->Digit3, (val1 % 10), (val2 % 10), (val3 % 10));

    return STATUS_CODE_OK;
  }
}

StatusCode seg_displays_set_float(SegDisplay *display, float val1, uint16_t val2, uint16_t val3) {
  uint16_t val_int = val1 * 10;

  if (val_int > MAX_DISPLAY_VALUE) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    seg_displays_reset(display);

    set_digit(display, &display->Digit1, (val_int / 100), (val2 / 100), (val3 / 100));

    gpio_set_state(&display->DP, GPIO_STATE_LOW);  // Enable decimal point
    set_digit(display, &display->Digit2, ((val_int / 10) % 10), ((val2 / 10) % 10),
              ((val3 / 10) % 10));

    gpio_set_state(&display->DP, GPIO_STATE_HIGH);
    set_digit(display, &display->Digit3, (val_int % 10), (val2 % 10), (val3 % 10));

    return STATUS_CODE_OK;
  }
}
