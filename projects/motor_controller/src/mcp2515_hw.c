
#include "mcp2515_hw.h"

#include "delay.h"
#include "gpio_it.h"
#include "log.h"
#include "mcp2515_defs.h"

// TX/RX buffer ID registers - See Registers 3-3 to 3-7, 4-4 to 4-8
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

typedef enum {
  MCP2515_FILTER_ID_RXF0 = 0,
  MCP2515_FILTER_ID_RXF1,
  NUM_MCP2515_FILTER_IDS
} Mcp2515FiltersIds;

typedef struct Mcp2515TxBuffer {
  uint8_t id;
  uint8_t data;
  uint8_t rts;
} Mcp2515TxBuffer;

typedef struct Mcp2515RxBuffer {
  uint8_t id;
  uint8_t data;
  uint8_t int_flag;
} Mcp2515RxBuffer;

// Storage
static Mcp2515Storage *s_storage;

static const Mcp2515TxBuffer s_tx_buffers[] = {
  { .id = MCP2515_LOAD_TXB0SIDH, .data = MCP2515_LOAD_TXB0D0, .rts = MCP2515_RTS_TXB0 },
  { .id = MCP2515_LOAD_TXB1SIDH, .data = MCP2515_LOAD_TXB1D0, .rts = MCP2515_RTS_TXB1 },
  { .id = MCP2515_LOAD_TXB2SIDH, .data = MCP2515_LOAD_TXB2D0, .rts = MCP2515_RTS_TXB2 },
};

static const Mcp2515RxBuffer s_rx_buffers[] = {
  { .id = MCP2515_READ_RXB0SIDH, .data = MCP2515_READ_RXB0D0, .int_flag = MCP2515_CANINT_RX0IE },
  { .id = MCP2515_READ_RXB1SIDH, .data = MCP2515_READ_RXB1D0, .int_flag = MCP2515_CANINT_RX1IE },
};

static const uint8_t s_brp_lookup[NUM_CAN_HW_BITRATES] = {
  // BRP calculated for different bitrates
  [CAN_HW_BITRATE_125KBPS] = MCP2515_CAN_BRP_125KBPS,
  [CAN_HW_BITRATE_250KBPS] = MCP2515_CAN_BRP_250KBPS,
  [CAN_HW_BITRATE_500KBPS] = MCP2515_CAN_BRP_500KBPS,
};

// SPI commands - See Table 12-1
static void prv_reset() {
  uint8_t payload[] = { MCP2515_CMD_RESET };
  spi_tx(s_storage->spi_port, payload, sizeof(payload));
}

static void prv_read(uint8_t addr, uint8_t *read_data, size_t read_len) {
  uint8_t payload[] = { MCP2515_CMD_READ, addr };
  spi_exchange(s_storage->spi_port, payload, sizeof(payload), read_data, read_len);
}

static void prv_write(uint8_t addr, uint8_t *write_data, size_t write_len) {
  uint8_t payload[MCP2515_MAX_WRITE_BUFFER_LEN];
  payload[0] = MCP2515_CMD_WRITE;
  payload[1] = addr;
  memcpy(&payload[2], write_data, write_len);

  spi_tx(s_storage->spi_port, payload, write_len + 2);
}

// See 12.10: *addr = (data & mask) | (*addr & ~mask)
static void prv_bit_modify(uint8_t addr, uint8_t mask, uint8_t data) {
  uint8_t payload[] = { MCP2515_CMD_BIT_MODIFY, addr, mask, data };

  spi_tx(s_storage->spi_port, payload, sizeof(payload));
}

static uint8_t prv_read_status() {
  uint8_t payload[] = { MCP2515_CMD_READ_STATUS };
  uint8_t read_data;
  spi_exchange(s_storage->spi_port, payload, sizeof(payload), &read_data, sizeof(read_data));

  return read_data;
}

static void prv_handle_rx(uint8_t buffer_id) {
  CanMessage rx_msg = { 0 };

  // bad code incoming:
  // uint8_t command = MCP2515_CMD_READ_RX | s_rx_buffers[buffer_id].id;
  // uint8_t *buffer = rx_msg.data_u8 - 5;
  // spi_exchange(s_storage->spi_port, &command, 1, buffer, 5 + 8);
  // Mcp2515IdRegs read_id_regs = {
  //   .registers = { buffer[3], buffer[2], buffer[1], buffer[0] },
  // };
  // LOG_DEBUG("%d, %d, %d, %d, %d\n", buffer[3], buffer[2], buffer[1], buffer[0], buffer[4]);
  // rx_msg.dlc = buffer[4] & 0xf;
  // rx_msg.extended = read_id_regs.extended;

  // if (!rx_msg.extended) {
  //   rx_msg.id.raw = read_id_regs.sid;
  // } else {
  //   rx_msg.id.raw = (uint32_t)(read_id_regs.sid << MCP2515_EXTENDED_ID_LEN) | read_id_regs.eid;
  // }
  // rx_msg.type = CAN_MSG_TYPE_DATA;
  // can_queue_push(&s_storage->rx_queue, &rx_msg);
  // bad code ended

  // Read ID and Data
  uint8_t command = MCP2515_CMD_READ_RX | s_rx_buffers[buffer_id].id;
  uint8_t data[5 + 8];  // id + data
  spi_exchange(s_storage->spi_port, &command, 1, data, sizeof(data));

  Mcp2515IdRegs read_id_regs = {
    .registers = { data[3], data[2], data[1], data[0] },
  };

  rx_msg.extended = read_id_regs.extended;
  rx_msg.dlc = data[4] & 0xf;

  if (!rx_msg.extended) {
    rx_msg.id.raw = read_id_regs.sid;
  } else {
    rx_msg.id.raw = (uint32_t)(read_id_regs.sid << MCP2515_EXTENDED_ID_LEN) | read_id_regs.eid;
  }
  memcpy(rx_msg.data_u8, &data[5], rx_msg.dlc);

  can_queue_push(&s_storage->rx_queue, &rx_msg);

  // LOG_DEBUG("complete rx %d id: %lx\n", buffer_id, rx_msg.id.raw);
}

static void prv_handle_error() {
  struct {
    uint8_t canintf;
    uint8_t eflg;
  } regs;

  prv_read(MCP2515_CTRL_REG_CANINTF, &regs.canintf, sizeof(regs));
  // LOG_DEBUG("handle error %x %x\n", regs.canintf, regs.eflg);
  // Clear flags
  if (regs.canintf & MCP2515_CANINT_EFLAG) {
    // Clear error flag
    prv_bit_modify(MCP2515_CTRL_REG_CANINTF, MCP2515_CANINT_EFLAG, 0);
  }

  if (regs.canintf & (MCP2515_EFLG_RX0_OVERFLOW | MCP2515_EFLG_RX1_OVERFLOW)) {
    // RX overflow - clear error flags
    // maybe trigger notification for rx
    prv_bit_modify(MCP2515_CTRL_REG_EFLG, MCP2515_EFLG_RX0_OVERFLOW | MCP2515_EFLG_RX1_OVERFLOW, 0);
  }

  s_storage->errors.eflg = regs.eflg;
  prv_read(MCP2515_CTRL_REG_TEC, &s_storage->errors.tec, 1);
  prv_read(MCP2515_CTRL_REG_REC, &s_storage->errors.rec, 1);

  if (regs.eflg) {
    // TODO: handle errors
    // s_storage->errors
  }
}

static CanHwBitrate mcp2515_bitrate;
static bool mcp2515_loopback;

static StatusCode mcp2515_hw_init_after_schedular_start() {
  prv_reset();
  // Set to Config mode, CLKOUT /4
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL,
                 MCP2515_CANCTRL_OPMODE_MASK | MCP2515_CANCTRL_CLKOUT_MASK,
                 MCP2515_CANCTRL_OPMODE_CONFIG | MCP2515_CANCTRL_CLKOUT_CLKPRE_4);

  // set RXB0CTRL.BUKT bit on to enable rollover to rx1
  prv_bit_modify(MCP2515_CTRL_REG_RXB0CTRL, MCP2515_RXB0CTRL_BUKT, MCP2515_RXB0CTRL_BUKT);
  // set RXnBF to be message buffer full interrupt
  prv_bit_modify(
      MCP2515_CTRL_REG_BFPCTRL,
      MCP2515_BFPCTRL_B1BFE | MCP2515_BFPCTRL_B2BFE | MCP2515_BFPCTRL_B1BFM | MCP2515_BFPCTRL_B2BFM,
      MCP2515_BFPCTRL_B1BFE | MCP2515_BFPCTRL_B2BFE | MCP2515_BFPCTRL_B1BFM |
          MCP2515_BFPCTRL_B2BFM);
  // 5.7 Timing configurations:
  // In order:
  // CNF3: PHSEG2 Length = 6
  // CNF2: PHSEG1 Length = 8, PRSEG Length = 1
  // CNF1: BRP = 0 (500kbps), 1 (250kbps), 3 (125kbps)
  // CANINTE: Enable error and receive interrupts
  // CANINTF: clear all IRQ flags
  // EFLG: clear all error flags
  const uint8_t s_registers[] = {
    0x05,
    MCP2515_CNF2_BTLMODE_CNF3 | MCP2515_CNF2_SAMPLE_3X | (0x07 << 3),
    s_brp_lookup[mcp2515_bitrate],
    MCP2515_CANINT_EFLAG,
    0x00,
    0x00,
  };
  prv_write(MCP2515_CTRL_REG_CNF3, s_registers, SIZEOF_ARRAY(s_registers));

  // Sanity check: read register after first write
  // If new reg value corresponds to expected
  uint8_t reg_val = 0;
  prv_read(MCP2515_CTRL_REG_CNF3, &reg_val, 1);

  LOG_DEBUG("MCP2515 Init Status:");
  if (reg_val == 0x05) {
    LOG_DEBUG("Connection SUCCESSFUL\n");
  } else {
    LOG_DEBUG("Connection UNSUCCESSFUL\n");
  }

  // Leave config mode
  uint8_t opmode =
      (mcp2515_loopback ? MCP2515_CANCTRL_OPMODE_LOOPBACK : MCP2515_CANCTRL_OPMODE_NORMAL);
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL, MCP2515_CANCTRL_OPMODE_MASK, opmode);

  return STATUS_CODE_OK;
}

TASK(MCP2515_INTERRUPT, TASK_STACK_256) {
  mcp2515_hw_init_after_schedular_start();

  LOG_DEBUG("MCP2515_INTERRUPT hw initilaized\n");

  uint32_t notification;
  while (true) {
    notify_wait(&notification, BLOCK_INDEFINITELY);

    // LOG_DEBUG("events: %lx\n", notification);

    if (notify_check_event(&notification, 0)) {  // Error interrupt
      // LOG_DEBUG("handle error\n");
      prv_handle_error();
    }
    if (notify_check_event(&notification, 1)) {  // RX0BF
      prv_handle_rx(0);
    }
    if (notify_check_event(&notification, 2)) {  // RX1BF
      prv_handle_rx(1);
    }
  }
}

// Initializes CAN using the specified settings.
StatusCode mcp2515_hw_init(Mcp2515Storage *storage, const Mcp2515Settings *settings) {
  s_storage = storage;
  // settings->spi_settings.mode = SPI_MODE_0;
  status_ok_or_return(spi_init(settings->spi_port, &settings->spi_settings));

  if (settings->can_settings.bitrate > CAN_HW_BITRATE_500KBPS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "mcp2515 does not support this bitrate");
  }

  mcp2515_bitrate = settings->can_settings.bitrate;
  mcp2515_loopback = settings->can_settings.loopback;

  s_storage->spi_port = settings->spi_port;
  s_storage->loopback = settings->can_settings.loopback;

  // active low
  status_ok_or_return(
      gpio_init_pin(&settings->interrupt_pin, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH));
  status_ok_or_return(gpio_init_pin(&settings->RX0BF, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH));
  status_ok_or_return(gpio_init_pin(&settings->RX1BF, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH));

  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .edge = INTERRUPT_EDGE_FALLING,
  };

  status_ok_or_return(
      gpio_it_register_interrupt(&settings->interrupt_pin, &it_settings, 0, MCP2515_INTERRUPT));
  status_ok_or_return(
      gpio_it_register_interrupt(&settings->RX0BF, &it_settings, 1, MCP2515_INTERRUPT));
  status_ok_or_return(
      gpio_it_register_interrupt(&settings->RX1BF, &it_settings, 2, MCP2515_INTERRUPT));

  // ! Ensure the task priority is higher than the rx/tx tasks in mcp2515.c
  status_ok_or_return(tasks_init_task(MCP2515_INTERRUPT, TASK_PRIORITY(5), NULL));

  return STATUS_CODE_OK;
}

StatusCode mcp2515_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended);

// CanHwBusStatus mcp2515_hw_bus_status(void);

StatusCode mcp2515_hw_transmit(uint32_t id, bool extended, uint8_t *data, size_t len) {
  // Ensure the CANCTRL register is set to the correct value
  //
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL, 0x1f, 0x0f);
  // Get free transmit buffer
  uint8_t status = prv_read_status();
  // LOG_DEBUG("stats: %x\n", status); //
  uint8_t tx_status = __builtin_ffs(
      ~status & (MCP2515_STATUS_TX0REQ | MCP2515_STATUS_TX1REQ | MCP2515_STATUS_TX2REQ));

  if (tx_status == 0) {
    LOG_DEBUG("Failed to tx, buffer full\n");
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }
  // LOG_DEBUG("message on buffer %d\n", (tx_status - 3) / 2);

  // Status format: 0b01010100 = all TXxREQ bits set
  // ffs returns 1-indexed: (x-3)/2 -> 0b00000111 = all TXxREQ bits set
  Mcp2515TxBuffer *tx_buf = &s_tx_buffers[(tx_status - 3) / 2];

  Mcp2515IdRegs tx_id_regs = { 0 };
  if (extended) {
    // extended
    tx_id_regs.sid = id >> MCP2515_STANDARD_ID_LEN;
    tx_id_regs.eid = id;
    tx_id_regs.extended = true;
  } else {
    // standard
    tx_id_regs.sid = id;
    tx_id_regs.extended = false;
  }

  // This payload buffer can be reused if needed for stack mem.
  // Load ID: SIDH, SIDL, EID8, EID0, RTSnDLC
  uint8_t id_payload[] = {
    MCP2515_CMD_LOAD_TX | tx_buf->id,  // tx_id_regs 3-3 to 3-6
    tx_id_regs.registers[3],          tx_id_regs.registers[2],
    tx_id_regs.registers[1],          tx_id_regs.registers[0],
    (len & 0xf) | (false << 6),  // Reg 3-7 RTR and DLC
  };
  spi_tx(s_storage->spi_port, id_payload, sizeof(id_payload));

  // Load data
  uint8_t data_payload[9] = { MCP2515_CMD_LOAD_TX | tx_buf->data };
  memcpy(&data_payload[1], data, len);

  spi_tx(s_storage->spi_port, data_payload, len + 1);

  // Send message
  uint8_t send_payload[] = { MCP2515_CMD_RTS | tx_buf->rts };
  spi_tx(s_storage->spi_port, send_payload, sizeof(send_payload));

  uint8_t data_test[] = { MCP2515_CMD_READ, 0x30 };
  uint8_t receive = 0;
  spi_exchange(SPI_PORT_2, data_test, 2, &receive, 1);
  // LOG_DEBUG("status: %x\n", receive);  // if 8, then its ok

  return STATUS_CODE_OK;
}

// Call with MCP2515 in Config mode to set filters
static void prv_configure_filters(CanMessageId *filters) {
  for (size_t i = 0; i < NUM_MCP2515_FILTER_IDS; i++) {
    // Prevents us from filtering for id 0x0
    if (filters[i] == 0) {
      continue;
    }
    Mcp2515IdRegs id_regs = { 0 };

    if (filters[i] >> MCP2515_STANDARD_ID_LEN) {
      // extended
      id_regs.extended = true;
      id_regs.sid = filters[i] >> MCP2515_EXTENDED_ID_LEN;
      id_regs.eid = filters[i];
    } else {
      // standard
      id_regs.extended = false;
      id_regs.sid = filters[i];
    }

    uint8_t maskRegH = (i == MCP2515_FILTER_ID_RXF0) ? MCP2515_REG_RXM0SIDH : MCP2515_REG_RXM1SIDH;
    size_t numMaskRegisters = id_regs.extended ? MCP2515_NUM_MASK_REGISTERS_EXTENDED
                                               : MCP2515_NUM_MASK_REGISTERS_STANDARD;
    // Set the filter masks to 0xff so we filter on the whole message
    for (size_t i = 0; i < numMaskRegisters; i++) {
      prv_bit_modify(maskRegH + i, 0xff, 0xff);
    }

    uint8_t filterRegH =
        (i == MCP2515_FILTER_ID_RXF0) ? MCP2515_REG_RXF0SIDH : MCP2515_REG_RXF1SIDH;
    // Set sidh
    prv_bit_modify(filterRegH, 0xff, id_regs.registers[3]);
    // Set sidl and eid16-17
    prv_bit_modify(filterRegH + 1, 0xff, id_regs.registers[2]);
    // Set eid8-15
    prv_bit_modify(filterRegH + 2, 0xff, id_regs.registers[1]);
    // Set eid0-7
    prv_bit_modify(filterRegH + 3, 0xff, id_regs.registers[0]);
  }
}

StatusCode mcp2515_hw_set_filter(CanMessageId *filters, bool loopback) {
  // Set to Config mode, CLKOUT /4
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL,
                 MCP2515_CANCTRL_OPMODE_MASK | MCP2515_CANCTRL_CLKOUT_MASK,
                 MCP2515_CANCTRL_OPMODE_CONFIG | MCP2515_CANCTRL_CLKOUT_CLKPRE_4);

  prv_configure_filters(filters);

  // Leave config mode
  uint8_t opmode = (loopback ? MCP2515_CANCTRL_OPMODE_LOOPBACK : MCP2515_CANCTRL_OPMODE_NORMAL);
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL, MCP2515_CANCTRL_OPMODE_MASK, opmode);
  return STATUS_CODE_OK;
}
