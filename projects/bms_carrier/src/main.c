#include <stdint.h>
#include <stdio.h>

#include "current_sense.h"
#include "gpio_it.h"
#include "log.h"
#include "master_task.h"
#include "max17261_fuel_gauge.h"
#include "tasks.h"

#define I2C2_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C2_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

#define FUEL_GAUGE_CYCLE_TIME_MS 100

static CurrentStorage s_currentsense_storage;

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {
  run_current_sense_cycle();
}

void run_slow_cycle() {}

#ifdef x86
int main(int argc, char *argv[]) {
  x86_main_init(atoi(argv[1]));
#else 
int main() {
#endif
  I2CSettings i2c_settings = { .sda = I2C2_SDA, .scl = I2C2_SCL, .speed = I2C_SPEED_STANDARD };

  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to BMS!\n");

  current_sense_init(&s_currentsense_storage, &i2c_settings, FUEL_GAUGE_CYCLE_TIME_MS);
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
