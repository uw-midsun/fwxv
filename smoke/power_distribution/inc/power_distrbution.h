#pragma once
#include "bts_load_switch.h"
#include "pca9555_gpio_expander.h"
#include "gpio.h"
#include "i2c.h"

#define I2C_SDA_PORT GPIO_PORT_B
#define I2C_SDA_PIN 9
#define I2C_SCL_PORT GPIO_PORT_B
#define I2C_SCL_PIN 8
// TODO: double check if these values are correct
#define I2C_ADDRESS_1 0x24
#define I2C_ADDRESS_2 0x26
// 4 to 1 mux for current sense
#define SENSE_MUX_SEL 4

#define CHECK_EQUAL(actual, expected)                         \
  ({                                                          \
    __typeof__(actual) a = (actual);                          \
    __typeof__(actual) e = (__typeof__(actual)) (expected);   \
    if (a != e) {                                             \
      LOG_DEBUG("Equal check failed!\n");                     \
    }                                                         \
  })

// UNCOMMENT FOR FRONT PD
#define FRONT_PD_BOARD
// UNCOMMENT FOR REAR PD
// #define REAR_PD_BOARD

#ifdef FRONT_PD_BOARD
  #define PD_BOARD "front"
#else
  #define PD_BOARD "rear"
#endif

typedef struct {
  Pca9555GpioAddress en_pca;
  Pca9555GpioAddress sel_pca;
  uint8_t select;
  uint8_t sense_select_code;
} PDSwitchConfig;


GpioAddress sense_pin = {
  .port = GPIO_PORT_A,
  .pin = 7
};

GpioAddress i2c_sda = {
  .port = I2C_SDA_PORT,
  .pin = I2C_SDA_PIN
};

GpioAddress i2c_scl = {
  .port = I2C_SCL_PORT,
  .pin = I2C_SCL_PIN
};

I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD, 
  .sda = {
    .port = I2C_SDA_PORT,
    .pin = I2C_SDA_PIN
  },
  .scl = {
  .port = I2C_SCL_PORT,
  .pin = I2C_SCL_PIN
  }
};

GpioAddress sense_mux_select_pins [] = {
  {.port = GPIO_PORT_A, .pin = 6},
  {.port = GPIO_PORT_A, .pin = 5},
  {.port = GPIO_PORT_A, .pin = 4},
  {.port = GPIO_PORT_A, .pin = 3},
};

#ifdef FRONT_PD_BOARD
  // front PD config

  // number of switches that use select pin
  #define NUM_SEL_SWITCHES 14

  // order switches with select first
  typedef enum {
    PEDAL = 0,
    STEERING,
    LEFT_REAR_CAMERA,
    RIGHT_CAMERA,
    SPARE_1,
    SPARE_2,
    CTR_CONSOLE,
    REAR_DISP,
    LEFT_DISP_CHARGE,
    RIGHT_DISP_STROBE,
    UV_CUTOFF,
    SPARE_3,
    FRONT_REAR_LEFT_TURN,
    FRONT_REAR_RIGHT_TURN,
    MAIN_PI_B,
    MAIN_DISP_BMS,
    DAYTIME_REAR_BRK,
    NUM_SWITCHES,
  } Switch;

  PDSwitchConfig conf [] = {
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_4 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_3 },
      .select = 0,
      .sense_select_code = 0x06
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_2 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_3 },
      .select = 1,
      .sense_select_code = 0x06
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_6 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_7 },
      .select = 0,
      .sense_select_code = 0x0c
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_7 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_7 },
      .select = 1,
      .sense_select_code = 0x0c
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_3 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_4 },
      .select = 0,
      .sense_select_code = 0x02
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_5 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_4 },
      .select = 1,
      .sense_select_code = 0x02
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_4 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_5 },
      .select = 0,
      .sense_select_code = 0x00
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_6 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_5 },
      .select = 1,
      .sense_select_code = 0x00
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_1 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_4 },
      .select = 0,
      .sense_select_code = 0x07
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_0 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_4 },
      .select = 1,
      .sense_select_code = 0x07
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_5 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_2 },
      .select = 0,
      .sense_select_code = 0x04
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_7 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_2 },
      .select = 1,
      .sense_select_code = 0x04
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_0 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_1},
      .select = 0,
      .sense_select_code = 0x09
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_2 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_1},
      .select = 1,
      .sense_select_code = 0x09
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_0 },
      .sense_select_code = 0x01
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_0 },
      .sense_select_code = 0x0d
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_3 },
      .sense_select_code = 0x08
    },
  };
#else
  // rear PD config

  // number of switches that use select pin
  #define NUM_SEL_SWITCHES 12

  // order switches with select first
  typedef enum {
    FAN1 = 0,
    FAN2,
    LEFT_REAR_CAMERA,
    RIGHT_CAMERA,
    SPARE_1,
    SPARE_2,
    LEFT_DISP_CHARGE,
    RIGHT_DISP_STROBE,
    UV_CUTOFF,
    SPARE_3,
    FRONT_REAR_LEFT_TURN,
    FRONT_REAR_RIGHT_TURN,
    MAIN_DISP_BMS,
    DAYTIME_REAR_BRK,
    SPEAKER_SOLAR_SENSE,
    MCI,
    NUM_SWITCHES,
  } Switch;

  PDSwitchConfig conf [] = {
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_7 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_6 },
      .select = 0,
      .sense_select_code = 0x0e
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_3 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_6 },
      .select = 1,
      .sense_select_code = 0x0e
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_6 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_7 },
      .select = 0,
      .sense_select_code = 0x0c
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_7 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_7 },
      .select = 1,
      .sense_select_code = 0x0c
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_3 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_4 },
      .select = 0,
      .sense_select_code = 0x02
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_5 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_4 },
      .select = 1,
      .sense_select_code = 0x02
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_1 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_4 },
      .select = 0,
      .sense_select_code = 0x07
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_0 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_4 },
      .select = 1,
      .sense_select_code = 0x07
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_5 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_2 },
      .select = 0,
      .sense_select_code = 0x04
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_7 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO0_2 },
      .select = 1,
      .sense_select_code = 0x04
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_0 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_1},
      .select = 0,
      .sense_select_code = 0x09
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_2 },
      .sel_pca = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_1},
      .select = 1,
      .sense_select_code = 0x09
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_0 },
      .sense_select_code = 0x0d
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_2, .pin = PCA9555_PIN_IO1_3 },
      .sense_select_code = 0x08
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_5 },
      .sense_select_code = 0x05
    },
    {
      .en_pca  = { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO0_6 },
      .sense_select_code = 0x03
    },
  };
#endif



Bts7xxxPin enable_pins[NUM_SWITCHES];
Bts7xxxPin select_pins[NUM_SEL_SWITCHES];
BtsLoadSwitchOutput switches[NUM_SWITCHES];
