#pragma once
// Definitions for all HW components on Centre Console
// TODO(mitchellostler): Update with actual addresses once schematic complete

// Centre Console LEDs 
  #define HAZARD_LED_ADDR \
    { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_4 }
  #define LEFT_LED_ADDR \ 
    { .i2c_address = 0x20, .pin = PCA9555_PIN_IO1_7 }
  #define RIGHT_LED_ADDR \ 
    { .i2c_address = 0x20, .pin = PCA9555_PIN_IO1_6 }
  #define CRUISE_LED_ADDR \ 
    { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_4 }
  #define REGEN_LED_ADDR \ 
    { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_5 }

// I/O expander connected to backlights
#define CC_IO_EXP_I2C_PORT NUM_I2C_PORTS
#define CC_IO_EXP_SDA \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_IO_EXP_SCL \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }

// Button Inputs and backlit LEDS
#define CC_BTN_PUSH_START \
  { .port = NUM_GPIO_PORTS - 1, .pin = GPIO_PINS_PER_PORT - 1 }
#define CC_BTN_STATE_DRIVE \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_BTN_STATE_NEUTRAL \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_BTN_STATE_REVERSE \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_BTN_DRL_LIGHTS \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_BTN_REGEN_BRAKE \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }

// LEDs for backlights on buttons
#define CC_LED_PUSH_START \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_STATE_DRIVE \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_STATE_NEUTRAL \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_STATE_REVERSE \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_STATE_REVERSE \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_DRL_LIGHTS \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_REGEN_BRAKE \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }

// Warning/indicator LEDS
#define CC_LED_TURN_SIGNAL_LEFT \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_TURN_SIGNAL_RIGHT \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_LED_BPS_FAULT_INDICATOR \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }

// Seven Segment display GPIO Addresses
// BATT display (V)
#define BATT_DISPLAY_BCD_A \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISPLAY_BCD_B \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISPLAY_BCD_C \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISPLAY_BCD_D \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISPLAY_DIGIT_SEL_1 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISPLAY_DIGIT_SEL_2 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISPLAY_DIGIT_SEL_3 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }

// Speedometer PWM config
#define SPDMTR_PWM_TIMER NUM_PWM_TIMERS
#define SPDMTR_PWM_PIN \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }

// Seven Segment Display GPIO Addresses for Centre Console
#define CC_DISP1 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_DISP2 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_DISP3 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_DISP4 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_DISP5 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_DISP6 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_DISP7 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_DISP8 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP1 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP2 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP3 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP4 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP5 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP6 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP7 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define SPD_DISP8 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP1 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP2 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP3 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP4 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP5 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP6 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP7 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define BATT_DISP8 \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
