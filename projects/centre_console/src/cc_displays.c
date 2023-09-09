#include "cc_displays.h"

#include "can.h"
#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "centre_console_tx_structs.h"
#include "gpio.h"
#include "log.h"
#include "seg_display.h"

// Multiplication Factor to convert CAN Velocity in 100 * m/s to kph
#define CONVERT_VELOCITY_TO_SPEED 0.018

SegDisplay cc_display = { .A = CC_DISP1,
                          .B = CC_DISP2,
                          .C = CC_DISP3,
                          .D = CC_DISP4,
                          .DP = CC_DISP5,
                          .D1 = CC_DISP6,
                          .D2 = CC_DISP7,
                          .D3 = CC_DISP8 };
SegDisplay speed_display = { .A = SPD_DISP1,
                             .B = SPD_DISP2,
                             .C = SPD_DISP3,
                             .D = SPD_DISP4,
                             .DP = SPD_DISP5,
                             .D1 = SPD_DISP6,
                             .D2 = SPD_DISP7,
                             .D3 = SPD_DISP8 };
SegDisplay batt_perc_display = { .A = BATT_DISP1,
                                 .B = BATT_DISP2,
                                 .C = BATT_DISP3,
                                 .D = BATT_DISP4,
                                 .DP = BATT_DISP5,
                                 .D1 = BATT_DISP6,
                                 .D2 = BATT_DISP7,
                                 .D3 = BATT_DISP8 };

void cc_displays_update(void) {
  seg_display_init(&cc_display);
  seg_display_init(&speed_display);
  seg_display_init(&batt_perc_display);
  // Read data from CAN structs and update displays with those values
  // TODO: Get this value from the source when we have the cruise_control value
  uint8_t cruise_control_val = g_tx_struct.drive_output_cruise_control;
  // Convert motor_velocity value to speed by averaging the right and left, dividing by 100, and
  // multiplying by 3.6 to get kph
  float speed_val = (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) *
                    CONVERT_VELOCITY_TO_SPEED;
  uint16_t batt_perc_val = get_battery_vt_batt_perc();
  seg_display_set_int(&cc_display, cruise_control_val);
  if (speed_val >= 100) {
    seg_display_set_int(&speed_display, (int)speed_val);
  } else {
    seg_display_set_float(&speed_display, speed_val);
  }
  seg_display_set_int(&batt_perc_display, batt_perc_val);
}
