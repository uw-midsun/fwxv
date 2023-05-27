#include <stdio.h>
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "spi.h"


// ==== WRITE PARAMETERS ====

//Set this to true to perform an SPI write
#define SHOULD_WRITE true

// Fill in these variables with the port and address to write to.
#define WRITE_SPI_PORT SPI_PORT_1 //Could be SPI_Port_2

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0x10, 0x2F };

// ==== READ PARAMETERS ====

// Set this to true to perform an SPI read.
#define SHOULD_READ true

// Fill in these variables with the port and address to read from.
#define READ_SPI_PORT SPI_PORT_1 

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 1

// ==== END OF PARAMETERS ====

static SpiSettings spi_settings = {
   .baudrate = 256, //TO FIND
   .mode = SPI_MODE_0,
   .mosi = { .port = GPIO_PORT_A, .pin = 4},
   .miso = { .port = GPIO_PORT_A, .pin = 5},
   .sclk = { .port = GPIO_PORT_A, .pin = 6},
   .cs = { .port = GPIO_PORT_A, .pin = 7},
};

TASK(smoke_spi_task, TASK_STACK_512) {
   spi_init(SPI_PORT_1, &spi_settings);
   while (true) {
      if(SHOULD_WRITE) {
         // Calculate the write length
         uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);

         StatusCode status;

         status = spi_tx(WRITE_SPI_PORT, bytes_to_write, tx_len);

         if (status == STATUS_CODE_OK) {
            LOG_DEBUG("Successfully wrote %d bytes on SPI_PORT_1\n", tx_len);
         }
         // Log an unsuccessful write
         if (status != STATUS_CODE_OK) {
            LOG_DEBUG("Write failed: status code %d\n", status);
         }
      }

      if(SHOULD_READ) {
         // Allocate space for the bytes we'll read
         uint8_t read_bytes[NUM_BYTES_TO_READ] = { 0 };
         uint8_t place[NUM_BYTES_TO_READ] = { 0 };
         uint8_t placeholder = { 0x0A };

         StatusCode status;

         status = spi_rx(READ_SPI_PORT, read_bytes, NUM_BYTES_TO_READ, placeholder);
         // Log a successful read
         LOG_DEBUG("Successfully read %d bytes on SPI_PORT_1\n",
                NUM_BYTES_TO_READ);

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
}

int main() {
   gpio_init();
   tasks_init();
   log_init();
   LOG_DEBUG("Welcome to TEST!");

   tasks_init_task(smoke_spi_task, TASK_PRIORITY(1), NULL);

   tasks_start();

   return 0;
}

