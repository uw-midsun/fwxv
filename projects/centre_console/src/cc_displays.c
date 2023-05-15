#include "cc_displays.h"

#include "can.h"
#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "centre_console_tx_structs.h"
#include "gpio.h"
#include "log.h"
#include "seg_display.h"

void cc_displays_update(void) {
  // Read data from CAN structs and update displays with those values
  uint8_t cruise_control_val = g_tx_struct.drive_output_cruise_control;
  // Convert motor_velocity value to speed by averaging the right and left, dividing by 100, and
  // multiplying by 3.6 to get kph
  float speed_val = (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) * 0.018;
  uint16_t batt_perc_val = get_battery_vt_batt_perc();
  SegDisplay cc_display = { CC_DISP1, CC_DISP2, CC_DISP3, CC_DISP4,
                            CC_DISP5, CC_DISP6, CC_DISP7, CC_DISP8 };
  SegDisplay speed_display = { SPD_DISP1, SPD_DISP2, SPD_DISP3, SPD_DISP4,
                               SPD_DISP5, SPD_DISP6, SPD_DISP7, SPD_DISP8 };
  SegDisplay batt_perc_display = { BATT_DISP1, BATT_DISP2, BATT_DISP3, BATT_DISP4,
                                   BATT_DISP5, BATT_DISP6, BATT_DISP7, BATT_DISP8 };
  seg_display_init(&cc_display);
  seg_display_init(&speed_display);
  seg_display_init(&batt_perc_display);
  seg_display_set_int(&cc_display, cruise_control_val);
  if (speed_val >= 100) {
    seg_display_set_int(&speed_display, (int)speed_val);
  } else {
    seg_display_set_float(&speed_display, speed_val);
  }
  seg_display_set_int(&batt_perc_display, batt_perc_val);
}
