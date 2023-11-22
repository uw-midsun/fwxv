#pragma once
// Definitions for all HW components on Centre Console
// I/O expander connected to backlights
#define CC_IO_EXP_ADDR 0x20

// Centre Console LEDs
#define CC_LED_LEFT \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_7 }
#define CC_LED_RIGHT \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_6 }
#define CC_LED_LIGHTS \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_5 }
#define CC_LED_CRUISE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_4 }

// Button Inputs and backlit LEDS
#define CC_BTN_POWER \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_1 }
#define CC_BTN_STATE_DRIVE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_0 }
#define CC_BTN_STATE_NEUTRAL \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_2 }
#define CC_BTN_STATE_REVERSE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_3 }
#define CC_BTN_HAZARD \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_7 }
#define CC_BTN_REGEN_BRAKE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_6 }

// LEDs for backlights on buttons
#define CC_LED_POWER \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_2 }
#define CC_LED_DRIVE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_3 }
#define CC_LED_NEUTRAL \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_1 }
#define CC_LED_REVERSE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_0 }
#define CC_LED_HAZARD \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_4 }
#define CC_LED_REGEN_BRAKE \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_6 }

// Seven Segment display GPIO Addresses
// Cruise Control
#define CC_DISP1 \
  { .port = GPIO_PORT_B, .pin = 11 } 
#define CC_DISP2 \
  { .port = GPIO_PORT_B, .pin = 1 }
#define CC_DISP3 \
  { .port = GPIO_PORT_B, .pin = 2 }
#define CC_DISP4 \
  { .port = GPIO_PORT_B, .pin = 10 }
#define CC_DISP5 \
  { .port = GPIO_PORT_A, .pin = 9 }
#define CC_DISP6 \
  { .port = GPIO_PORT_B, .pin = 14 }
#define CC_DISP7 \
  { .port = GPIO_PORT_B, .pin = 15 }
#define CC_DISP8 \
  { .port = GPIO_PORT_A, .pin = 8 }
#define CC_DISPLAY                                                                      \
  {                                                                                       \
    .A = CC_DISP1, .B = CC_DISP2, .C = CC_DISP3, .D = CC_DISP4, .DP = CC_DISP5, \
    .D1 = CC_DISP6, .D2 = CC_DISP7, .D3 = CC_DISP8                                  \
  }

#define SPD_DISP1 \
  { .port = GPIO_PORT_A, .pin = 3 }
#define SPD_DISP2 \
  { .port = GPIO_PORT_A, .pin = 0 }
#define SPD_DISP3 \
  { .port = GPIO_PORT_A, .pin = 1 }
#define SPD_DISP4 \
  { .port = GPIO_PORT_A, .pin = 2 }
#define SPD_DISP5 \
  { .port = GPIO_PORT_A, .pin = 9 }
#define SPD_DISP6 \
  { .port = GPIO_PORT_B, .pin = 14 }
#define SPD_DISP7 \
  { .port = GPIO_PORT_B, .pin = 15 }
#define SPD_DISP8 \
  { .port = GPIO_PORT_A, .pin = 8 }

#define SPD_DISPLAY                                                                      \
  {                                                                                       \
    .A = SPD_DISP1, .B = SPD_DISP2, .C = SPD_DISP3, .D = SPD_DISP4, .DP = SPD_DISP5, \
    .D1 = SPD_DISP6, .D2 = SPD_DISP7, .D3 = SPD_DISP8                                  \
  }

#define BATT_DISP1 \
  { .port = GPIO_PORT_A, .pin = 7 }
#define BATT_DISP2 \
  { .port = GPIO_PORT_A, .pin = 4 }
#define BATT_DISP3 \
  { .port = GPIO_PORT_A, .pin = 5 }
#define BATT_DISP4 \
  { .port = GPIO_PORT_A, .pin = 6 }
#define BATT_DISP5 \
  { .port = GPIO_PORT_A, .pin = 9 }
#define BATT_DISP6 \
  { .port = GPIO_PORT_B, .pin = 14 }
#define BATT_DISP7 \
  { .port = GPIO_PORT_B, .pin = 15 }
#define BATT_DISP8 \
  { .port = GPIO_PORT_A, .pin = 8 }

#define BATT_DISPLAY                                                                      \
  {                                                                                       \
    .A = BATT_DISP1, .B = BATT_DISP2, .C = BATT_DISP3, .D = BATT_DISP4, .DP = BATT_DISP5, \
    .D1 = BATT_DISP6, .D2 = BATT_DISP7, .D3 = BATT_DISP8                                  \
  }
