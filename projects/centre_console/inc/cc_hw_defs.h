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
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_4 }
#define REGEN_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_5 }
#define LIGHTS_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO1_5 }
#define POWER_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_2 }
#define BPS_LED_ADDR \
  { .i2c_address = CC_IO_EXP_ADDR, .pin = PCA9555_PIN_IO0_2 }

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

// Speedometer
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

// Battery
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

// All displays
#define ALL_DISPLAYS                                                                              \
  {                                                                                               \
    .A1 = SPD_DISP1, .B1 = SPD_DISP2, .C1 = SPD_DISP3, .D1 = SPD_DISP4, .A2 = BATT_DISP1,         \
    .B2 = BATT_DISP2, .C2 = BATT_DISP3, .D2 = BATT_DISP4, .A3 = CC_DISP1, .B3 = CC_DISP2,         \
    .C3 = CC_DISP3, .D3 = CC_DISP4, .DP = BATT_DISP5, .Digit1 = BATT_DISP6, .Digit2 = BATT_DISP7, \
    .Digit3 = BATT_DISP8                                                                          \
  }
