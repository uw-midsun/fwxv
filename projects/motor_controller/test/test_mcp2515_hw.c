#include "mcp2515.h"
#include "test_helpers.h"
#include "unity.h"

StatusCode TEST_MOCK(spi_init)(SpiPort spi, const SpiSettings *settings) {
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  return STATUS_CODE_OK;
}

void setup_test() {
  // mcp2515_hw_init();
}

void teardown_test() {}

void test_mcp2515(void) {}

void test_tx(void) {}

void test_rx(void) {}
