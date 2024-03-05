#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include "i2c.h"
#include "current_sense.h"
#include "delay.h"

#define FUEL_GAUGE_CYCLE_TIME_MS 100
#define BMS_PERIPH_I2C_SDA_PIN \
  { GPIO_PORT_B, 11 }
#define BMS_PERIPH_I2C_SCL_PIN \
  { GPIO_PORT_B, 10 }
#define BMS_FAN_ALERT_PIN \
  { GPIO_PORT_A, 9 }

static CurrentStorage s_currentsense_storage;
static const I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = BMS_PERIPH_I2C_SDA_PIN,
  .scl = BMS_PERIPH_I2C_SCL_PIN,
};

TASK(current_sense_task, TASK_STACK_256)
{
  while (true)
  {
    prv_fuel_gauge_read();
    delay_ms(1000);
  }
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  current_sense_init(&s_currentsense_storage, &i2c_settings, FUEL_GAUGE_CYCLE_TIME_MS);

  tasks_init_task(current_sense_task, 1, NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
