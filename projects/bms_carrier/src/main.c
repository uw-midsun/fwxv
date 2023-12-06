#include <stdio.h>
#include <stdint.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

#include "gpio_it.h"
#include "current_sense.h"
#include "max17261_fuel_gauge.h"

#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 9 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 8 }

#define FUEL_GAUGE_CYCLE_TIME_MS 100

static CurrentStorage s_currentsense_storage;

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  I2CSettings i2c_settings = {
    .sda = I2C1_SDA,
    .scl = I2C1_SCL,
    .speed = I2C_SPEED_STANDARD
  };

  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to BMS!\n");

  current_sense_init(&s_currentsense_storage, &i2c_settings, FUEL_GAUGE_CYCLE_TIME_MS);
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
