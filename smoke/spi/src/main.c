#include <stdio.h>

#include "gpio.h"
#include "log.h"
#include "spi.h"
#include "tasks.h"

// ==== WRITE PARAMETERS ====

// Set this to true to perform an SPI write
#define SHOULD_WRITE true

// Fill in these variables with the port and address to write to.
#define WRITE_SPI_PORT SPI_PORT_1  // Could be SPI_Port_2

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0xFC };

// ==== READ PARAMETERS ====

// Set this to true to perform an SPI read.
#define SHOULD_READ true

// Fill in these variables with the port and address to read from.
#define READ_SPI_PORT SPI_PORT_1

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 1

// ==== END OF PARAMETERS ====

static SpiSettings spi_settings = {
  .baudrate = 1000000,  // TO FIND
  .mode = SPI_MODE_0,
  .mosi = { .port = GPIO_PORT_A, .pin = 7 },
  .miso = { .port = GPIO_PORT_A, .pin = 6 },
  .sclk = { .port = GPIO_PORT_A, .pin = 5 },
  .cs = { .port = GPIO_PORT_A, .pin = 4 },
};

TASK(smoke_spi_task, TASK_STACK_512) {
  spi_init(SPI_PORT_1, &spi_settings);
  while (true) {
    uint8_t read_bytes[NUM_BYTES_TO_READ] = { 0 };
    StatusCode status = spi_exchange(READ_SPI_PORT, bytes_to_write, SIZEOF_ARRAY(bytes_to_write),
                                     read_bytes, NUM_BYTES_TO_READ);
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Successfully did a read and write cycle.\n");
    }
  }
}

int main() {
  gpio_init();
  tasks_init();
  log_init();
  // LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(smoke_spi_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}
