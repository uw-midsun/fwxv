#pragma once
// Definitions for all HW components on Centre Console
// I/O expander connected to backlights
#define CC_IO_EXP_ADDR 0x20
#define CC_IO_EXP_I2C_PORT NUM_I2C_PORTS
#define CC_IO_EXP_SDA \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }
#define CC_IO_EXP_SCL \
  { .port = NUM_GPIO_PORTS, .pin = GPIO_PINS_PER_PORT }

// Centre Console LEDs
#define HAZARD_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define LEFT_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_7 }
#define RIGHT_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_6 }
#define CRUISE_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define REGEN_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_5 }

// Button Inputs and backlit LEDS
#define CC_BTN_POWER \
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
#define CC_LED_POWER \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define CC_LED_STATE_DRIVE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define CC_LED_STATE_NEUTRAL \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define CC_LED_STATE_REVERSE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define CC_LED_STATE_REVERSE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define CC_LED_REGEN_BRAKE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }

// Seven Segment display GPIO Addresses
// Cruise Control
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
#define CC_DISPLAY                                                                              \
  {                                                                                             \
    .A = CC_DISP1, .B = CC_DISP2, .C = CC_DISP3, .D = CC_DISP4, .DP = CC_DISP5, .D1 = CC_DISP6, \
    .D2 = CC_DISP7, .D3 = CC_DISP8                                                              \
  }

// Speed
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
#define SPD_DISPLAY                                                                             \
  {                                                                                             \
    .A = CC_DISP1, .B = CC_DISP2, .C = CC_DISP3, .D = CC_DISP4, .DP = CC_DISP5, .D1 = CC_DISP6, \
    .D2 = CC_DISP7, .D3 = CC_DISP8                                                              \
  }

// Battery
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
#define BATT_DISPLAY                                                                      \
  {                                                                                       \
    .A = BATT_DISP1, .B = BATT_DISP2, .C = BATT_DISP3, .D = BATT_DISP4, .DP = BATT_DISP5, \
    .D1 = BATT_DISP6, .D2 = BATT_DISP7, .D3 = BATT_DISP8                                  \
  }
