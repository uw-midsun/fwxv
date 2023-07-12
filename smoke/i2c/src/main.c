#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "tasks.h"
#include "log.h"
#include "delay.h"

// ==== WRITE PARAMETERS ====

//Set this to true to perform an I2C write.
#define SHOULD_WRITE true

// Fill in these variables with the port and address to write to.
#define WRITE_I2C_PORT I2C_PORT_1 //Could be I2C_Port_2
#define WRITE_I2C_ADDRESS 0x08

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0x10, 0x2F };

// Set this to true to write to a register or false to write normally.
#define SHOULD_WRITE_REGISTER false

// If the previous parameter is true, fill in this variable with the register to write to.
#define REGISTER_TO_WRITE 0x06

// ==== READ PARAMETERS ====

// Set this to true to perform an I2C read.
#define SHOULD_READ false

// Fill in these variables with the port and address to read from.
#define READ_I2C_PORT I2C_PORT_1 
#define READ_I2C_ADDRESS 0x08

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 1

// Set this to true to read from a register or false to read normally.
#define SHOULD_READ_REGISTER true

// If the previous parameter is true, fill in this variable with the register to read from.
#define REGISTER_TO_READ 0x06

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
  .sda = { .port = GPIO_PORT_B, .pin = 9 }, 
  .scl = { .port = GPIO_PORT_B, .pin = 8 }, 
};

static const GpioAddress test_pin = {
    .pin = 3,
    .port = GPIO_PORT_B
};

TASK(smoke_i2c_task, TASK_STACK_512){
  uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);
  uint8_t rx_buf[8] = {0};
  i2c_init(I2C_PORT_1, &i2c_settings);
  gpio_init_pin(&test_pin ,GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  while (true) {
    // i2c_write(WRITE_I2C_PORT, WRITE_I2C_ADDRESS, bytes_to_write, tx_len);
    i2c_read(READ_I2C_PORT, READ_I2C_ADDRESS, rx_buf, 6);
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