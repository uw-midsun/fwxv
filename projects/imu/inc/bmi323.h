#pragma once

#include "log.h"
#include "spi.h"


#define CMD 0x7E
#define WRITE 0x7F
#define READ 0x80
#define FEATURE_IO0 0x10
#define FEATURE_IO1 0x11
#define FEATURE_IO2 0x12
#define FEATURE_IO3 0x13
#define FEATURE_IO_STATUS 0x14
#define FEATURE_CTRL 0x40
#define INT_STATUS_INT1 0x0D
#define INT_STATUS_INT2 0x0E



#define DUMMY_BYTE 0x00
#define BMI_SET_LOW_BYTE 0x00FF
#define BMI_SET_HIGH_BYTE 0xFF00
#define BMI3_E_NULL_PTR INT8_C(-1)



/*
this might be 13 or 14 idk
#define BMI3_ACC_DP_OFF_XYZ_13_BIT_MASK              UINT16_C(0x1FFF)
#define BMI3_ACC_DP_OFF_XYZ_14_BIT_MASK              UINT16_C(0x3FFF)
*/
#define BMI3_ACC_DP_DOFFSET_X_MASK UINT16_C(0xFFFF)
#define BMI3_ACC_DP_DOFFSET_Y_MASK UINT16_C(0xFFFF)
#define BMI3_ACC_DP_DOFFSET_Z_MASK UINT16_C(0xFFFF)

#define BMI3_ACC_DP_DGAIN_X_MASK UINT16_C(0x00FF)
#define BMI3_ACC_DP_DGAIN_Y_MASK UINT16_C(0x00FF)
#define BMI3_ACC_DP_DGAIN_Z_MASK UINT16_C(0x00FF)

#define BMI3_GYR_DP_OFF_X_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_X_MASK UINT16_C(0x007F)
#define BMI3_GYR_DP_OFF_Y_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_Y_MASK UINT16_C(0x007F)
#define BMI3_GYR_DP_OFF_Z_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_Z_MASK UINT16_C(0x007F)

typedef enum{
  REG_BANK_SEL = 0x7F,
  GYRO_REG_ADDR = 0x06,
  ACCEL_REG_ADDR = 0x03
}bmi323_registers;

typedef enum {
    ACC_DP_OFF_X = 0x60,
    ACC_DP_DGAIN_X = 0x61,
    ACC_DP_OFF_Y = 0x62,
    ACC_DP_DGAIN_Y = 0x63,
    ACC_DP_OFF_Z = 0x64,
    ACC_DP_DGAIN_Z = 0x65
}accel_go_registers;

typedef enum {
    GYR_DP_OFF_X = 0x66,
    GYR_DP_DGAIN_X = 0x67,
    GYR_DP_OFF_Y = 0x68,
    GYR_DP_DGAIN_Y = 0x69,
    GYR_DP_OFF_Z = 0x6A,
    GYR_DP_DGAIN_Z = 0x6B
}gyro_go_registers;

typedef struct{
  uint16_t accel_offset_x;
  uint16_t accel_offset_y;
  uint16_t accel_offset_z;
  uint8_t accel_gain_x;
  uint8_t accel_gain_y;
  uint8_t accel_gain_z;
}accel_gain_offset_values;

typedef struct{
  uint16_t gyro_offset_x;
  uint16_t gyro_offset_y;
  uint16_t gyro_offset_z;
  uint8_t gyro_gain_x;
  uint8_t gyro_gain_y;
  uint8_t gyro_gain_z;
}gyro_gain_offset_values;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} axes;

typedef struct {
  bmi323_settings *settings;
  axes accel;
  axes gyro;
  accel_gain_offset_values accel_go_values;
  gyro_gain_offset_values gyro_go_values;
} bmi323_storage;

typedef struct{
  SpiPort spi_port;
  uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
  GpioAddress int1;
  GpioAddress int2;
}bmi323_settings;

static StatusCode set_register(uint16_t reg_addr, uint16_t value);
static StatusCode set_multi_register(uint8_t reg_addr, uint8_t *value, uint16_t len);
static StatusCode get_register(bmi323_registers reg, uint8_t *value);
static StatusCode get_multi_register(bmi323_registers reg, uint8_t *reg_val, uint8_t len);
static StatusCode get_gyroscope_data(axes *gyro);
static StatusCode get_accel_data(axes *accel);
static StatusCode enable_feature_engine();