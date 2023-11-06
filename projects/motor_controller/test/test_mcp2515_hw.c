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

typedef union {
  struct {
    uint32_t eid : 18;
    uint8_t _ : 1;
    uint8_t extended : 1;
    uint8_t srr : 1;
    uint32_t sid : 11;
  };
  uint8_t registers[4];
} Mcp2515IdRegs;

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
  for (size_t i = 0; i < rx_len; ++i) {
    TEST_ASSERT_OK(queue_receive(&spi_rx_data, rx_data + i, 1));
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

bool initialized = false;
void setup_test() {
  if (initialized) return;
  initialized = true;

  gpio_init();
  interrupt_init();
  gpio_it_init();
  callback_init(1);

  can_queue_init(&s_storage.rx_queue);
  queue_init(&spi_tx_data);
  queue_init(&spi_rx_data);

  StatusCode code = mcp2515_hw_init(&s_storage, &s_mcp2515_settings);
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

  // timing config
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

  // sanity check
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CNF3, data);
  data = 0x05;
  queue_send(&spi_rx_data, &data, 0);

  // Leave config mode
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANCTRL, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
}

bool transmit_message(void *data) {
  CanMessage *message = (CanMessage *)data;
  status_ok_or_return(
      mcp2515_hw_transmit(message->id.raw, message->extended, message->data_u8, message->dlc));
  return true;
}

TEST_IN_TASK
void test_tx(void) {
  queue_reset(&spi_tx_data);
  queue_reset(&spi_rx_data);

  CanMessage message = {
    .extended = false,
    .id.raw = 641,  // 1241,
    .dlc = 8,
    .data_u8 = { 0, 1, 2, 3, 4, 5, 6, 7 },
  };
  // run transmit_message(&message); async
  Event event = register_callback(transmit_message, &message);
  notify(callback_task, event);
  delay_ms(1);

  uint8_t data;
  // Ensure the CANCTRL register is set to the correct value
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANCTRL, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));

  // read status
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ_STATUS, data);
  data = 0x00;  // all buffer free
  queue_send(&spi_rx_data, &data, 0);

  // id
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_LOAD_TX | MCP2515_LOAD_TXB0SIDH, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(message.id.raw >> 3, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL((message.id.raw & 7) << 5, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(message.dlc, data & 0xf);

  // data
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_LOAD_TX | MCP2515_LOAD_TXB0D0, data);
  for (size_t i = 0; i < message.dlc; ++i) {
    TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
    TEST_ASSERT_EQUAL(message.data_u8[i], data);
  }

  // send message (rts)
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_RTS | MCP2515_RTS_TXB0, data);
}

TEST_IN_TASK
void test_rx(void) {
  queue_reset(&spi_tx_data);
  queue_reset(&spi_rx_data);

  uint8_t data;
  // trigger RX0BF gpio pin interrupt, expect an spi_exchange
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_mcp2515_settings.RX0BF));

  // id
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ_RX | MCP2515_READ_RXB0SIDH, data);
  Mcp2515IdRegs id_regs = { .sid = 641 };
  data = 8;  // dlc
  queue_send(&spi_rx_data, &id_regs.registers[3], 0);
  queue_send(&spi_rx_data, &id_regs.registers[2], 0);
  queue_send(&spi_rx_data, &id_regs.registers[1], 0);
  queue_send(&spi_rx_data, &id_regs.registers[0], 0);
  queue_send(&spi_rx_data, &data, 0);

  // data
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ_RX | MCP2515_READ_RXB0D0, data);
  uint8_t message_data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  for (int i = 0; i < 8; ++i) {
    queue_send(&spi_rx_data, &message_data[i], 0);
  }

  // interrupt flag clear
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 10));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANINTF, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CANINT_RX0IE, data);
  TEST_ASSERT_OK(queue_receive(&spi_tx_data, &data, 1));
  TEST_ASSERT_EQUAL(0, data);

  // assert message
  CanMessage message;
  TEST_ASSERT_OK(queue_receive(&s_storage.rx_queue.queue, &message, 10));

  TEST_ASSERT_EQUAL(641, message.id.raw);
  TEST_ASSERT_EQUAL(8, message.dlc);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(message_data, message.data_u8, 8);
}
