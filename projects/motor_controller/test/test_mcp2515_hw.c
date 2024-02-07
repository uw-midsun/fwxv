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
}

void teardown_test() {}

TEST_IN_TASK
void test_mcp2515_init_after_schedular_start(void) {
  uint8_t data[10];

  // reset
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_RESET, data[0]);

  // Set to Config mode, CLKOUT /4
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 4));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data[0]);
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANCTRL, data[1]);

  // set RXB0CTRL.BUKT bit on to enable rollover to rx1
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 4));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data[0]);
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_RXB0CTRL, data[1]);

  // set RXnBF to be message buffer full interrupt
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 4));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data[0]);
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_BFPCTRL, data[1]);

  // timing config
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 8));
  TEST_ASSERT_EQUAL(MCP2515_CMD_WRITE, data[0]);
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CNF3, data[1]);

  // sanity check
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 2));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ, data[0]);
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CNF3, data[1]);
  data[0] = 0x05;
  spi_set_rx(SPI_PORT_2, data, 1);

  // Leave config mode
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 4));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data[0]);
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANCTRL, data[1]);
}

bool transmit_message(void *data) {
  CanMessage *message = (CanMessage *)data;
  status_ok_or_return(
      mcp2515_hw_transmit(message->id.raw, message->extended, message->data_u8, message->dlc));
  return true;
}

TEST_IN_TASK
void test_tx(void) {
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

  uint8_t data[10];
  // Ensure the CANCTRL register is set to the correct value
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 4));
  TEST_ASSERT_EQUAL(MCP2515_CMD_BIT_MODIFY, data[0]);
  TEST_ASSERT_EQUAL(MCP2515_CTRL_REG_CANCTRL, data[1]);

  // read status
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ_STATUS, data[0]);
  data[0] = 0x00;  // all buffer free
  spi_set_rx(SPI_PORT_2, data, 1);

  // id
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 6));
  TEST_ASSERT_EQUAL(MCP2515_CMD_LOAD_TX | MCP2515_LOAD_TXB0SIDH, data[0]);
  TEST_ASSERT_EQUAL(message.id.raw >> 3, data[1]);
  TEST_ASSERT_EQUAL((message.id.raw & 7) << 5, data[2]);
  TEST_ASSERT_EQUAL(message.dlc, data[5] & 0xf);

  // data
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 1 + message.dlc));
  TEST_ASSERT_EQUAL(MCP2515_CMD_LOAD_TX | MCP2515_LOAD_TXB0D0, data[0]);
  for (size_t i = 0; i < message.dlc; ++i) {
    TEST_ASSERT_EQUAL(message.data_u8[i], data[i + 1]);
  }

  // send message (rts)
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, data, 1));
  TEST_ASSERT_EQUAL(MCP2515_CMD_RTS | MCP2515_RTS_TXB0, data[0]);
}

TEST_IN_TASK
void test_rx(void) {
  uint8_t data;
  // trigger RX0BF gpio pin interrupt, expect an spi_exchange
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_mcp2515_settings.RX0BF));

  // id and data
  Mcp2515IdRegs id_regs = { .sid = 641 };
  uint8_t regs_data[] = {
    id_regs.registers[3],
    id_regs.registers[2],
    id_regs.registers[1],
    id_regs.registers[0],
    8,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
  };
  TEST_ASSERT_OK(spi_get_tx(SPI_PORT_2, &data, 1));
  spi_set_rx(SPI_PORT_2, regs_data, sizeof(regs_data));
  TEST_ASSERT_EQUAL(MCP2515_CMD_READ_RX | MCP2515_READ_RXB0SIDH, data);

  // assert message
  CanMessage message;
  TEST_ASSERT_OK(queue_receive(&s_storage.rx_queue.queue, &message, 100));

  TEST_ASSERT_EQUAL(641, message.id.raw);
  TEST_ASSERT_EQUAL(8, message.dlc);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&regs_data[5], message.data_u8, 8);
}
