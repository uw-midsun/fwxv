#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "master_task.h"
#include "delay.h"
#include "pca_config.h"
#include "gpio.h"


Pca9555GpioAddress addy[] = {
   {.i2c_address = I2C_ADDRESS_1, .pin = PCA9555_PIN_IO1_0},
   // add more addresses to test as needed.
};

Pca9555GpioSettings pca_settings = {
   .direction = GPIO_OUTPUT_PUSH_PULL,
   .state = GPIO_STATE_LOW
};


TASK(master_task, TASK_MIN_STACK_SIZE) {
   size_t num_pins = sizeof(addy) / sizeof(Pca9555GpioAddress); 
   for(unsigned int i = 0; i < num_pins; ++i) {
      pca9555_gpio_init_pin(&addy[i], &pca_settings);
   }
   while (true) {
      uint8_t tx_data = 0xff;
      // for(int i = 0; i < 8; i++) {
      //   gpio_toggle_state(&sda);
      // }
      i2c_write(I2C_PORT_1, 0x24, &tx_data, 1);
      // for(unsigned int i = 0; i < num_pins; ++i) {
      //    pca9555_gpio_toggle_state(&addy[i]);
      // }
      // for(unsigned int i = 0; i < num_pins; ++i) {
      //    pca9555_gpio_set_state(&addy[i], PCA9555_GPIO_STATE_HIGH);
      // }
      // delay_ms(200);
      // for(unsigned int i = 0; i < num_pins; ++i) {
      //    pca9555_gpio_set_state(&addy[i], PCA9555_GPIO_STATE_LOW);
      // }
      delay_ms(5000);
   }
}

int main() {
   tasks_init();
   log_init();
   gpio_init();
   //gpio_it_init();
   GpioAddress sda = {.port = i2c_settings.scl.port, .pin = i2c_settings.scl.pin };
   gpio_init_pin(&sda, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

   i2c_init(0, &i2c_settings);
   pca9555_gpio_init(0, I2C_ADDRESS_1);

   LOG_DEBUG("Welcome to TEST!");

   tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   LOG_DEBUG("exiting main?");
   return 0;
}

