#include "cc_hw_defs.h"
#include "pca9555_gpio_expander.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "centre_console_tx_structs.h"
#include "steering_setters.h"
#include "gpio.h"
#include "log.h"
#include "seg_display.h"
#include "can.h"
#include "power_fsm.h"

//We need a module that keeps track of statuses and sets/resets the following:
// Indicators for left/right turn signals and hazards
// Regen Braking indicator
// BPS Fault indicator
// The functionality is based on information either received in can messages from other boards, 
// or button presses recorded on the current board. 

#define DRIVE_OUTPUT_REGEN_BRAKING g_tx_struct.drive_output_regen_braking

// CruiseControl Mask Values for each state
typedef enum {
  CC_DECREASE_MASK = 0x01,
  CC_INCREASE_MASK = 0x02,
  CC_TOGGLE_MASK = 0x04,
} CruiseControlMask;

typedef enum {
    REGEN_TOGGLE_MASK = 0x04
} RegenMask;

#define NUM_DRIVE_LED 5

typedef enum DriveLeds {
  HAZARD_LED = 0, 
  LEFT_LED,
  RIGHT_LED,
  CRUISE_LED,
  REGEN_LED,
} DriveLeds;

static Pca9555GpioAddress s_drive_btn_leds[NUM_DRIVE_LED] = {
  [HAZARD_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_4 },
  [LEFT_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO1_7 },
  [RIGHT_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO1_6 },
  [CRUISE_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_4 },
  [REGEN_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_5 }
};

// Cruise control toggle
// regen braking
// Left signal
// Right Signal
// Hazard
StatusCode update_indicators(bool hazard_info ) {
    uint8_t cruise_control_info =  get_steering_input_cc();
    uint8_t signal_info = get_steering_input_lights();
    uint8_t hazard_info =  get_hazard_state();
    uint8_t bps_fault = 0; // TODO: add BPS message 
    uint8_t regen_braking_info =  DRIVE_OUTPUT_REGEN_BRAKING;
    
    if (hazard_info) {
        pca9555_gpio_set_state(&s_drive_btn_leds[CRUISE_LED], PCA9555_GPIO_STATE_HIGH);
    } else {
        pca9555_gpio_set_state(&s_drive_btn_leds[CRUISE_LED], PCA9555_GPIO_STATE_HIGH);
    }

    // don't know the amount of bytes for certain conditions, ask Mitch. Where do I find the values for certain conditions?
    if (cruise_control_info & CC_DECREASE_MASK) {
        pca9555_gpio_set_state(&s_drive_btn_leds[CRUISE_LED], PCA9555_GPIO_STATE_HIGH);
    } else if (cruise_control_info & CC_INCREASE_MASK) {
        pca9555_gpio_set_state(&s_drive_btn_leds[CRUISE_LED], PCA9555_GPIO_STATE_HIGH);
    } else if (cruise_control_info & CC_TOGGLE_MASK) {
        pca9555_gpio_set_state(&s_drive_btn_leds[CRUISE_LED], PCA9555_GPIO_STATE_LOW);
    }

    if (regen_braking_info & REGEN_TOGGLE_MASK) {
        pca9555_gpio_set_state(&s_drive_btn_leds[REGEN_LED], PCA9555_GPIO_STATE_HIGH);
    } else {
        pca9555_gpio_set_state(&s_drive_btn_leds[REGEN_LED], PCA9555_GPIO_STATE_LOW);
    }

if ()
    



}







