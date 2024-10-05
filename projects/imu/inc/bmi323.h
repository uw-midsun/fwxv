#pragma once

#include "spi.h"

#define READ 0x80
#define WRITE 0x7F

typedef enum{
  REG_BANK_SEL = 0x7F,


}bmi323_registers;

typedef struct {
//   icm20948_gyro_range gyro_range;
//   icm20948_accel_range accel_range;
//   ak09916_mode mag_mode;

  SpiPort spi_port;
//   const SpiMode spi_mode;
//   const uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} bmi323_settings;

typedef struct {
  bmi323_settings *settings;
} bmi323_storage;