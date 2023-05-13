#include "cc_displays.h"

// #include "can.h"
#include "cc_hw_defs.h"
#include "gpio.h"
#include "log.h"
// #include "seg_display.h"

void cc_displays_update(void) {
  static const GpioAddress cc_displays[] = {
    GPIO1,  GPIO2,  GPIO3,  GPIO4,  GPIO5,  GPIO6,  GPIO7,  GPIO8,  GPIO9,  GPIO10, GPIO11,
    GPIO12, GPIO13, GPIO14, GPIO15, GPIO16, GPIO17, GPIO18, GPIO19, GPIO20, GPIO21,
  };
  for (uint8_t i = 0; i < SIZEOF_ARRAY(cc_displays); i++) {
    gpio_init_pin(&cc_displays[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  }
  // Read from CAN structs and update displays
  uint8_t cruise_control_val = g_tx_struct.drive_output_cruise_control;
  float speed_val =
      (g_rx_struct.motor_velocity.velocity_l + g_rx_struct.motor_velocity.velocity_r) * 0.005;
  uint8_t batt_perc = g_rx_struct.battery_vt_batt_perc;
  SegDisplay cc_display = { GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7 };
  SegDisplay speed_display = { GPIO8, GPIO9, GPIO10, GPIO11, GPIO12, GPIO13, GPIO14 };
  SegDisplay batt_perc_display = { GPIO15, GPIO16, GPIO17, GPIO18, GPIO19, GPIO20, GPIO21 };
  seg_display_set_int(&cc_display, cruise_control_val);
  if (speed_val >= 100) {
    seg_display_set_int(&speed_display, (int)speed_val);
  } else {
    seg_display_set_float(&speed_display, speed_val);
  }
  seg_display_set_int(&batt_perc_display, batt_perc);
}