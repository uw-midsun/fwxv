#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "tasks.h"
#include "log.h"
#include "delay.h"

// ==== WRITE PARAMETERS ====

// Fill in these variables with the port and address to write to.
#define WRITE_I2C_PORT I2C_PORT_1 // I2C_Port_2 is also available
#define WRITE_I2C_ADDRESS 0x08

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0x10, 0x2F };

// ==== READ PARAMETERS ====

// Fill in these variables with the port and address to read from.
#define READ_I2C_PORT I2C_PORT_1 
#define READ_I2C_ADDRESS 0x08

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 1

// ==== END OF PARAMETERS ====

// These are the SDA and SCL ports for the I2C ports 1 and 2.
#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 9 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 8 }
#define I2C2_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C2_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

static I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD,  
  .sda = I2C1_SDA, 
  .scl = I2C1_SCL, 
};

static const GpioAddress test_pin = {
    .pin = 3,
    .port = GPIO_PORT_B
};

TASK(smoke_i2c_task, TASK_STACK_512){
  uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);
  uint8_t rx_buf[SIZEOF_ARRAY(bytes_to_write)] = {0};
  i2c_init(I2C_PORT_1, &i2c_settings);
  gpio_init_pin(&test_pin ,GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  while (true) {
    // I2C write
    i2c_write(WRITE_I2C_PORT, WRITE_I2C_ADDRESS, bytes_to_write, tx_len);
    // I2C read (uncomment to test)
    // i2c_read(READ_I2C_PORT, READ_I2C_ADDRESS, rx_buf, 6);
    gpio_toggle_state(&test_pin);
    delay_ms(100);
  }
}

int main(void) {
  gpio_init();
  tasks_init();
  log_init();

  tasks_init_task(smoke_i2c_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}