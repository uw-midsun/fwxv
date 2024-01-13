#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "master_task.h"
#include "i2c.h"
#include "max17261_fuel_gauge.h"

void pre_loop_init() 
{
}

void run_fast_cycle()
{
  LOG_DEBUG("fast cycle\n");
}

void run_medium_cycle()
{
  LOG_DEBUG("med cycle\n");
}

void run_slow_cycle()
{
}

int main() {
  log_init();
  tasks_init();

  I2CPort i2c_port = I2C_PORT_2;
  I2CSettings i2c_settings = { .scl = { .port = GPIO_PORT_B, .pin = 10 },
                               .sda = { .port = GPIO_PORT_B, .pin = 11 },
                               .speed = I2C_SPEED_STANDARD };
  uint8_t i2c_address = 0x36;

  StatusCode ret = i2c_init(i2c_port, &i2c_settings);
  if(ret != STATUS_CODE_OK){
    LOG_DEBUG("i2c_init status code (%d) \n", ret);
    return -1;
  }

  Max17261Storage storage;
  Max17261Settings settings = {
    .i2c_port = I2C_PORT_2,
    .i2c_address = 0x36,
    .design_capacity = 0,      // LSB = 5.0 (micro Volt Hours / R Sense)
    .empty_voltage = 0,        // Only a 9-bit field, LSB = 78.125 (micro Volts)
    .charge_term_current = 0,  // LSB = 1.5625 (micro Volts / R Sense)
    .r_sense_uohms = 500
  };
  storage.settings = settings;

  uint16_t dev_name = 0x0;
  ret = max17261_get_reg(&storage, MAX17261_DEV_NAME, &dev_name);
  if(ret != STATUS_CODE_OK){
    LOG_DEBUG("status code (%d) \n", ret);
    return -1;
  }
  LOG_DEBUG("dev_name=%#04x\n", dev_name);

  set_master_cycle_time(1000);
  init_master_task();

  tasks_start();
  LOG_DEBUG("exiting main?");
  return 0;
}

