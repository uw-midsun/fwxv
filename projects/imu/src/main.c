#include <stdio.h>

#include "gpio.h"
#include "icm20948.h"
#include "log.h"
#include "master_task.h"
#include "spi.h"
#include "tasks.h"

icm20948_settings imu_settings = {
  .spi_port = SPI_PORT_2,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .spi_baudrate = 2500000,

  .gyro_range = ICM20948_GYRO_RANGE_2000_DPS,
  .accel_range = ICM20948_ACCEL_RANGE_16_G,
  .mag_mode = CONTINUOUS_MEASUREMENT_10HZ,
};

icm20948_storage imu_storage = {
  .accel = { 0 },
  .gyro = { 0 },
  .mag = { 0 },
  .settings = &imu_settings,
};

void pre_loop_init() {
  icm20948_init(&imu_storage, &imu_settings);
}

void run_fast_cycle() {}

void run_medium_cycle() {
  icm20948_getAccelData();
  LOG_DEBUG("ACCEL X: %d, %d, %d\n", imu_storage.accel.x, imu_storage.accel.y, imu_storage.accel.z);

  icm20948_getGyroData();
  LOG_DEBUG("GYRO X: %d, %d, %d\n", imu_storage.gyro.x, imu_storage.gyro.y, imu_storage.gyro.z);

  if (!icm20948_getMagData()) {
    LOG_DEBUG("MAG X: %d, %d, %d\n", imu_storage.mag.x, imu_storage.mag.y, imu_storage.mag.z);
  }
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  gpio_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  init_master_task();
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
