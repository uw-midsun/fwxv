#pragma once

#include "log.h"
#include "spi.h"


#define READ 0x80
#define WRITE 0x7F
#define DUMMY_BYTE = 0x00
#define BMI_SET_LOW_BYTE = UINT16_C(0x00FF)
#define BMI_SET_HIGH_BYTE = UINT16_C(0xFF00)


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
}accel_og;

typedef enum {
    GYR_DP_OFF_X = 0x66,
    GYR_DP_DGAIN_X = 0x67,
    GYR_DP_OFF_Y = 0x68,
    GYR_DP_DGAIN_Y = 0x69,
    GYR_DP_OFF_Z = 0x6A,
    GYR_DP_DGAIN_Z = 0x6B
}gyro_og;

struct accel_gain_offset{
  uint16_t accel_offset_x;
  uint16_t accel_offset_y;
  uint16_t accel_offset_z;
  uint16_t accel_gain_x;
  uint16_t accel_gain_y;
  uint16_t accel_gain_z;
};

typedef struct {
  SpiPort spi_port;
}bmi323_settings;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} axes;

typedef struct {
  bmi323_settings *settings;
  axes accel;
  axes gyro;
} bmi323_storage;


static StatusCode set_register(uint16_t reg_addr, uint16_t value);
static StatusCode set_multi_register(uint16_t reg_addr, uint16_t *value, uint16_t len);
static StatusCode get_register(bmi323_registers reg, uint16_t *value);
static StatusCode get_multi_register(bmi323_registers reg, uint16_t *reg_val[6], uint8_t len);

static StatusCode get_gyroscope_data(axes *gyro);