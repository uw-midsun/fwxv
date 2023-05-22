#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "tasks.h"
#include "log.h"

// ==== WRITE PARAMETERS ====

//Set this to true to perform an I2C write.
#define SHOULD_WRITE true

// Fill in these variables with the port and address to write to.
#define WRITE_I2C_PORT I2C_PORT_1 //Could be I2C_Port_2
#define WRITE_I2C_ADDRESS 0x74

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0x10, 0x2F };

// Set this to true to write to a register or false to write normally.
#define SHOULD_WRITE_REGISTER true

// If the previous parameter is true, fill in this variable with the register to write to.
#define REGISTER_TO_WRITE 0x06

// ==== READ PARAMETERS ====

// Set this to true to perform an I2C read.
#define SHOULD_READ true

// Fill in these variables with the port and address to read from.
#define READ_I2C_PORT I2C_PORT_1 
#define READ_I2C_ADDRESS 0x74

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
  .speed = I2C_SPEED_FAST,  
  .sda = { .port = GPIO_PORT_B, .pin = 11 }, 
  .scl = { .port = GPIO_PORT_B, .pin = 10 }, 
};

TASK(smoke_i2c_task, TASK_STACK_512){
  i2c_init(I2C_PORT_1, &i2c_settings);
  while (true) {
  if (SHOULD_WRITE) {
    // Calculate the write length
    uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);

    StatusCode status;
    if (SHOULD_WRITE_REGISTER) {
      // Perform the write to a register
      status = i2c_write_reg(WRITE_I2C_PORT, WRITE_I2C_ADDRESS, REGISTER_TO_WRITE, bytes_to_write,
                             tx_len);

      // Log a successful write
      if (status == STATUS_CODE_OK) {
        LOG_DEBUG("Successfully wrote %d bytes to register %x at I2C address %x on I2C_PORT_1\n",
                  tx_len, REGISTER_TO_WRITE, WRITE_I2C_ADDRESS);
      }
    } else {
      // Perform the write normally
      status = i2c_write(WRITE_I2C_PORT, WRITE_I2C_ADDRESS, bytes_to_write, tx_len);

      // Log a successful write
      if (status == STATUS_CODE_OK) {
        LOG_DEBUG("Successfully wrote %d bytes to I2C address %x on I2C_PORT_1\n", tx_len,
                  WRITE_I2C_ADDRESS);
      }
    }

    // Log an unsuccessful write
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Write failed: status code %d\n", status);
    }
  }
}

  if (SHOULD_READ) {
    // Allocate space for the bytes we'll read
    uint8_t read_bytes[NUM_BYTES_TO_READ] = { 0 };

    StatusCode status;
    if (SHOULD_READ_REGISTER) {
      // Perform the read from a register
      status = i2c_read_reg(READ_I2C_PORT, READ_I2C_ADDRESS, REGISTER_TO_READ, read_bytes,
                            NUM_BYTES_TO_READ);

      // Log a successful read
      LOG_DEBUG("Successfully read %d bytes from register %x at I2C address %x on I2C_PORT_1\n",
                NUM_BYTES_TO_READ, REGISTER_TO_READ, READ_I2C_ADDRESS);
    } else {
      // Perform the read normally
      status = i2c_read(READ_I2C_PORT, READ_I2C_ADDRESS, read_bytes, NUM_BYTES_TO_READ);

      // Log a successful read
      LOG_DEBUG("Successfully read %d bytes from I2C address %x on I2C_PORT_1\n",
                NUM_BYTES_TO_READ, READ_I2C_ADDRESS);
    }

    if (status == STATUS_CODE_OK) {
      // Log the bytes we read
      for (uint16_t i = 0; i < NUM_BYTES_TO_READ; i++) {
        LOG_DEBUG("Byte %x read: %x\n", i, read_bytes[i]);
      }
    } else {
      // Log an unsucessful read
      LOG_DEBUG("Read failed: status code %d\n", status);
    }
  }  
}

int main(void) {
  tasks_init();
  log_init();

  tasks_init_task(smoke_i2c_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}
