#include "callback_handler.h"
#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "queues.h"
#include "spi.h"
#include "status.h"
#include "stdint.h"
#include "test_helpers.h"
#include "unity.h"

Mcp2515Storage s_storage;
static Mcp2515Settings s_mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .spi_settings = {
    .baudrate = 10000000,  // 10 Mhz
    .mode = SPI_MODE_0,
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_B, 12 },
  },
  .interrupt_pin = { GPIO_PORT_A, 8 },
  .RX0BF = { GPIO_PORT_B, 10 },
  .RX1BF = { GPIO_PORT_B, 11 },
  .can_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .loopback = true,
  },
};

bool initialized = false;
void setup_test() {
  if (initialized) return;
  initialized = true;

  gpio_init();
  interrupt_init();
  gpio_it_init();
  callback_init(1);

  can_queue_init(&s_storage.rx_queue);

  StatusCode code = mcp2515_hw_init(&s_storage, &s_mcp2515_settings);
  TEST_ASSERT_OK(code);

  LOG_DEBUG("SETUP COMPLETE\n");
}

void teardown_test() {}

TEST_IN_TASK
void test_all(void) {
  LOG_DEBUG("STARTING TEST\n");
  CanMessage message = {
    .extended = false,
    .id.raw = 641,  // 1241,
    .dlc = 8,
    .data_u8 = { 0, 1, 2, 3, 4, 5, 6, 7 },
  };

  while (true) {
    LOG_DEBUG("SEND MESSAGE\n");
    mcp2515_hw_transmit(message.id.raw, message.extended, message.data_u8, message.dlc);
    delay_s(5);
  }
}
