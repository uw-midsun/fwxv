#include <stdio.h>

#include "delay.h"
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
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

TASK(smoke_spi_task, TASK_STACK_512) {
  spi_init(SPI_PORT_2, &spi_settings);
  uint8_t spi_tx_data[5] = { 0x02, 0x28, 0x7, 0x8, 0x9 };
  while (true) {
    spi_cs_set_state(SPI_PORT_2, GPIO_STATE_LOW);
    uint8_t read_bytes[3] = { 0 };
    StatusCode status = spi_exchange(SPI_PORT_2, spi_tx_data, SIZEOF_ARRAY(spi_tx_data), NULL, 0);
    spi_cs_set_state(SPI_PORT_2, GPIO_STATE_HIGH);

    uint8_t read_cmd[] = { 0x03, 0x28 };
    spi_cs_set_state(SPI_PORT_2, GPIO_STATE_LOW);
    status = spi_exchange(SPI_PORT_2, read_cmd, 2, read_bytes, 3);
    // status = spi_tx(SPI_PORT_2, read_cmd, 2);
    // StatusCode status2 = spi_rx(SPI_PORT_2, read_bytes, SIZEOF_ARRAY(read_bytes), 0x0);
    LOG_DEBUG("STATUS %d Data: %x, %x, %x\n", status, read_bytes[0], read_bytes[1], read_bytes[2]);
    spi_cs_set_state(SPI_PORT_2, GPIO_STATE_HIGH);
    delay_ms(1000);
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
