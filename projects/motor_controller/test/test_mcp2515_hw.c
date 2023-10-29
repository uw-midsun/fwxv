#include "gpio_it.h"
#include "mcp2515.h"
#include "queues.h"
#include "status.h"
#include "stdint.h"
#include "test_helpers.h"
#include "unity.h"

StatusCode TEST_MOCK(spi_init)(SpiPort spi, const SpiSettings *settings) {
  // TODO: assert spi port is correct, settings are correct
  return STATUS_CODE_OK;
}

uint8_t spi_tx_data_buf[64];
Queue spi_tx_data = {
  .storage_buf = spi_tx_data_buf,
};
StatusCode TEST_MOCK(spi_tx)(SpiPort spi, uint8_t *tx_data, size_t tx_len) {
  queue_send(&spi_tx_data, tx_data, 0);
  return STATUS_CODE_OK;
}

Queue spi_rx_data;
StatusCode TEST_MOCK(spi_rx)(SpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  return STATUS_CODE_OK;
}

void setup_test() {
  // mcp2515_hw_init();
  queue_init(&spi_tx_data);
  queue_init(&spi_rx_data);
}

void teardown_test() {}

void test_mcp2515(void) {}

void test_tx(void) {}

void test_rx(void) {
  // trigger RX0BF gpio pin interrupt, expect an spi_exchange

  gpio_it_trigger_interrupt();
}
