#include <stdio.h>
#include "can_board_ids.h"
#include "log.h"
#include "master_task.h"
#include "mppt.h"
#include "tasks.h"
#include "temp_sense.h"

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {
  read_mppts();
}

void run_slow_cycle() {}

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_SOLAR_SENSE,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
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
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to Solar Sense!");
  adc_init();
  can_init(&s_can_storage, &can_settings);
  temp_sense_adc_init();
  mppt_init(&spi_settings, SPI_PORT_2);

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
