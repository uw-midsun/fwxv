#include "sd.h"

#define SPI_PORT SPI_PORT_1

SpiSettings spi_settings = { .baudrate = 25000000,  // 25 MHz
                             .mode = SPI_MODE_0,

                             .mosi = { .port = GPIO_PORT_B, .pin = 15 },
                             .miso = { .port = GPIO_PORT_B, .pin = 14 },
                             .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                             .cs = { .port = GPIO_PORT_B, .pin = 12 } };

// Puts SD card in spi mode
sd_init() {
  spi_init(SPI_PORT_1, &spi_settings);

  gpio_set_state(spi_settings.cs, GPIO_STATE_LOW);

  // Send CMD 0 (TO-DO)

  gpio_set_state(&spi_settings.cs, GPIO_STATE_HIGH);

  // Receive SPI mode R1 response
}