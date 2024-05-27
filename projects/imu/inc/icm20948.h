#pragma once

#include "delay.h"
#include "gpio.h"
#include "spi.h"
#include "status.h"

#define ACCELERATION_DUE_TO_GRAVITY_MM 9807  // In units of mm/s^2

// The accelerometer data range
typedef enum {
  ICM20948_ACCEL_RANGE_2_G = 0x00,
  ICM20948_ACCEL_RANGE_4_G,
  ICM20948_ACCEL_RANGE_8_G,
  ICM20948_ACCEL_RANGE_16_G,
} icm20948_accel_range;

// The gyro data range
typedef enum {
  ICM20948_GYRO_RANGE_250_DPS = 0x00,
  ICM20948_GYRO_RANGE_500_DPS,
  ICM20948_GYRO_RANGE_1000_DPS,
  ICM20948_GYRO_RANGE_2000_DPS,
} icm20948_gyro_range;

typedef enum {
  POWER_DOWN_MODE = 0,
  SINGLE_MEASUREMENT_MODE = 1,
  CONTINUOUS_MEASUREMENT_10HZ = 2,
  CONTINUOUS_MEASUREMENT_20HZ = 4,
  CONTINUOUS_MEASUREMENT_50HZ = 6,
  CONTINUOUS_MEASUREMENT_100HZ = 8
} ak09916_mode;

typedef struct {
  icm20948_gyro_range gyro_range;
  icm20948_accel_range accel_range;
  ak09916_mode mag_mode;

  SpiPort spi_port;
  const SpiMode spi_mode;
  const uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} icm20948_settings;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} axises;

typedef struct {
  axises accel;
  axises gyro;
  axises mag;

  icm20948_settings *settings;
} icm20948_storage;

// Retrieves accel x/y/z data
StatusCode icm20948_getAccelData();

// Retrieves gyro x/y/z data
StatusCode icm20948_getGyroData();

// Retrieves mag x/y/z data
StatusCode icm20948_getMagData();

// Initializes the icm20948 IMU. I2C is initialized inside this function
StatusCode icm20948_init(icm20948_storage *storage, icm20948_settings *settings);
