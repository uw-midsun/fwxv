#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "sd_binary.h"
#include "spi.h"
#include "tasks.h"

static SpiSettings spi_settings = {
  .baudrate = 10000000,  // 10 Mhz
  .mode = SPI_MODE_0,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

TASK(smoke_sd_task, TASK_STACK_512) {
  StatusCode s;
  spi_init(SPI_PORT_2, &spi_settings);
  sd_card_init(SPI_PORT_2);
  uint8_t spi_tx_data[512] = malloc(sizeof(uint8_t) * 512);
  uint8_t spi_rx_data[512] = malloc(sizeof(uint8_t) * 512);
  while (true) {
    s = sd_read_blocks(SPI_PORT_2, spi_rx_data, 0, 1);
    LOG_DEBUG("Read Operation Status %s\n", s);
    LOG_DEBUG("bytes READ: \n");
    for (int j = 0; j < 64; j++) {
      LOG_DEBUG("%x %x %x %x %x %x %x %x\n", spi_rx_data[j + 0], spi_rx_data[j + 1],
                spi_rx_data[j + 2], spi_rx_data[j + 3], spi_rx_data[j + 4], spi_rx_data[j + 5],
                spi_rx_data[j + 6], spi_rx_data[j + 7]);
    }
    delay_ms(1000);
    s = sd_write_blocks(SPI_PORT_2, spi_tx_data, 0, 1);
    LOG_DEBUG("Write Operation Status %s\n", s);
    LOG_DEBUG("bytes Write: \n");
    for (int j = 0; j < 64; j++) {
      LOG_DEBUG("%x %x %x %x %x %x %x %x\n", spi_tx_data[j + 0], spi_tx_data[j + 1],
                spi_tx_data[j + 2], spi_tx_data[j + 3], spi_tx_data[j + 4], spi_tx_data[j + 5],
                spi_tx_data[j + 6], spi_tx_data[j + 7]);
    }
    delay_ms(1000);
  }
  free(spi_tx_data);
  free(spi_rx_data);
}

int main() {
  gpio_init();
  tasks_init();
  log_init();

  tasks_init_task(smoke_sd_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}
