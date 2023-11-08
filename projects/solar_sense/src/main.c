#include "can.h"
#include "log.h"
#include "mppt.h"
#include "master_task.h"

void run_fast_cycle() {}

void run_medium_cycle() {
  read_mppts();
}

void run_slow_cycle() {}

static CanStorage s_can_storage = { 0 };
static const CanSettings s_can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
  .mode = CAN_CONTINUOUS,
};

static const SpiSettings spi_settings = {
  .baudrate = 1000000,
  .mode = SPI_MODE_3,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

int main() {
  log_init();

  LOG_DEBUG("Welcome to Solar Sense!");

  can_init(&s_can_storage, &s_can_settings);
  mppt_init(&spi_settings, SPI_PORT_2);
  tasks_init();

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
