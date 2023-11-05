#include "delay.h"
#include "gpio_it.h"
#include "log.h"
#include "mcp2515.h"
#include "queues.h"
#include "spi.h"
#include "status.h"
#include "stdint.h"
#include "test_helpers.h"
#include "unity.h"

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

typedef struct SpiMessage {
  size_t len;
  uint8_t data[MCP2515_MAX_WRITE_BUFFER_LEN];
} SpiMessage;

StatusCode TEST_MOCK(spi_init)(SpiPort spi, const SpiSettings *settings) {
  // TODO: assert spi port is correct, settings are correct
  TEST_ASSERT_EQUAL(SPI_PORT_2, spi);
  TEST_ASSERT_EQUAL(10000000, settings->baudrate);
  TEST_ASSERT_EQUAL(SPI_MODE_0, settings->mode);

  TEST_ASSERT_EQUAL(GPIO_PORT_B, settings->mosi.port);
  TEST_ASSERT_EQUAL(15, settings->mosi.pin);
  TEST_ASSERT_EQUAL(GPIO_PORT_B, settings->miso.port);
  TEST_ASSERT_EQUAL(14, settings->miso.pin);
  TEST_ASSERT_EQUAL(GPIO_PORT_B, settings->sclk.port);
  TEST_ASSERT_EQUAL(13, settings->sclk.pin);
  TEST_ASSERT_EQUAL(GPIO_PORT_B, settings->cs.port);
  TEST_ASSERT_EQUAL(12, settings->cs.pin);

  return STATUS_CODE_OK;
}

uint8_t spi_tx_data_buf[64];
Queue spi_tx_data = {
  .num_items = 64,
  .item_size = 1,
  .storage_buf = spi_tx_data_buf,
};
StatusCode TEST_MOCK(spi_tx)(SpiPort spi, uint8_t *tx_data, size_t tx_len) {
  printf("TX CALLED\n");
  for (size_t i = 0; i < tx_len; ++i) {
    queue_send(&spi_tx_data, tx_data + i, 0);
  }
  return STATUS_CODE_OK;
}

uint8_t spi_rx_data_buf[64];
Queue spi_rx_data = {
  .num_items = 64,
  .item_size = 1,
  .storage_buf = spi_rx_data_buf,
};
StatusCode TEST_MOCK(spi_rx)(SpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  printf("RX XALLED\n");
  for (size_t i = 0; i < rx_len; ++i) {
    TEST_ASSERT_OK(queue_receive(&spi_rx_data, rx_data + i, 1));
    printf("received %d\n", *rx_data);
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }
  spi_cs_set_state(spi, GPIO_STATE_LOW);
  spi_tx(spi, tx_data, tx_len);
  spi_rx(spi, rx_data, rx_len, 0x00);
  spi_cs_set_state(spi, GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}

Mcp2515Storage s_storage;

bool initialized = false;
void setup_test() {
  if (initialized) return;
  initialized = true;

  gpio_init();
  gpio_it_init();

  can_queue_init(&s_storage.rx_queue);
  queue_init(&spi_tx_data);
  queue_init(&spi_rx_data);

  StatusCode code = mcp2515_hw_init(&s_storage, &s_mcp2515_settings);
  LOG_DEBUG("%s\n", status_get().message);
  TEST_ASSERT_OK(code);
}

void teardown_test() {}

TEST_IN_TASK
void test_mcp2515_init_after_schedular_start(void) {
  uint8_t data;
  // reset
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_RESET, data);
  // Set to Config mode, CLKOUT /4
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANCTRL, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  // set RXB0CTRL.BUKT bit on to enable rollover to rx1
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_RXB0CTRL, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  // set RXnBF to be message buffer full interrupt
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_BFPCTRL, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));

  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_WRITE, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CNF3, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));

  data = 0x05;
  queue_send(&spi_rx_data, &data, 0);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CNF3, data);

  // Leave config mode
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANCTRL, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
}

TEST_IN_TASK
void test_mcp2515(void) {}

TEST_IN_TASK
void test_tx(void) {}

TEST_IN_TASK
void test_rx(void) {
  // trigger RX0BF gpio pin interrupt, expect an spi_exchange

  // gpio_it_trigger_interrupt();
}
