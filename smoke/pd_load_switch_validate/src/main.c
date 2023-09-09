#include <stdio.h>

#include "delay.h"
#include "log.h"
#include "master_task.h"
#include "pca_config.h"
#include "tasks.h"

Pca9555GpioAddress addy[] = {
  { .i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_0 },
  // add more addresses to test as needed.
};
static I2CSettings settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = { .port = GPIO_PORT_B, .pin = 9 },
  .scl = { .port = GPIO_PORT_B, .pin = 8 },
};
uint8_t data[] = { 0x2, 0x1F, 0xAF };
Pca9555GpioSettings pca_settings = { .direction = PCA9555_GPIO_DIR_IN, .state = GPIO_STATE_LOW };

TASK(master_task, TASK_MIN_STACK_SIZE) {
  size_t num_pins = sizeof(addy) / sizeof(Pca9555GpioAddress);
  //   for (unsigned int i = 0; i < num_pins; ++i) {
  // pca9555_gpio_init_pin(&addy[0], &pca_settings);
  //   }
  while (true) {
    // Try to turn on all the PCA Expander pins
    uint8_t address = 0x24;
    i2c_write(I2C_PORT_1, address, data, SIZEOF_ARRAY(data));
    // for (unsigned int i = 0; i < num_pins; ++i) {
    //   pca9555_gpio_set_state(&addy[i], PCA9555_GPIO_STATE_HIGH);
    // }
    // for(unsigned int i = 0; i < num_pins; ++i) {
    //    pca9555_gpio_set_state(&addy[i], PCA9555_GPIO_STATE_HIGH);
    // }
    // delay_ms(200);
    // for(unsigned int i = 0; i < num_pins; ++i) {
    //    pca9555_gpio_set_state(&addy[i], PCA9555_GPIO_STATE_LOW);
    // }
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  gpio_it_init();

  i2c_init(I2C_PORT_1, &settings);
  pca9555_gpio_init(0, I2C_ADDRESS_1);

  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
