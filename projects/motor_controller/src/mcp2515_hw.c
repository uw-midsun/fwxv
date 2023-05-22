
#include "mcp2515_hw.h"

#include "delay.h"
#include "gpio_it.h"
#include "log.h"
#include "mcp2515_defs.h"

typedef struct Mcp2515LoadTxPayload {
  uint8_t cmd;
  uint64_t data;
} __attribute__((packed)) Mcp2515LoadTxPayload;

// TX/RX buffer ID registers - See Registers 3-3 to 3-7, 4-4 to 4-8
typedef struct Mcp2515IdRegs {
  uint8_t sidh;
  union {
    struct {
      uint8_t eid_16_17 : 2;
      uint8_t unimplemented : 1;
      uint8_t ide : 1;
      uint8_t srr : 1;
      uint8_t sid_0_2 : 3;
    };
    uint8_t raw;
  } sidl;
  uint8_t eid8;
  uint8_t eid0;
  union {
    struct {
      uint8_t dlc : 4;
      uint8_t reserved : 2;
      uint8_t rtr : 1;
      uint8_t unimplemented : 1;
    };
    uint8_t raw;
  } dlc;
} Mcp2515IdRegs;

typedef union Mcp2515Id {
  struct {
    uint32_t eid0 : 8;
    uint32_t eid8 : 8;
    uint32_t eid_16_17 : 2;
    uint32_t sid_0_2 : 3;
    uint32_t sidh : 8;
    uint32_t padding : 3;
  };
  uint32_t raw;
} Mcp2515Id;

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
  spi_exchange(s_storage->spi_port, payload, sizeof(payload), NULL, 0);
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
  spi_exchange(s_storage->spi_port, payload, sizeof(payload), write_data, write_len);
}

// See 12.10: *addr = (data & mask) | (*addr & ~mask)
static void prv_bit_modify(uint8_t addr, uint8_t mask, uint8_t data) {
  uint8_t payload[] = { MCP2515_CMD_BIT_MODIFY, addr, mask, data };
  spi_exchange(s_storage->spi_port, payload, sizeof(payload), NULL, 0);
}

static uint8_t prv_read_status() {
  uint8_t payload[] = { MCP2515_CMD_READ_STATUS };
  uint8_t read_data[1] = { 0 };
  spi_exchange(s_storage->spi_port, payload, sizeof(payload), read_data, sizeof(read_data));

  return read_data[0];
}

static void prv_handle_rx(uint8_t int_flags) {
  CanMessage rx_msg = { 0 };
  for (size_t i = 0; i < SIZEOF_ARRAY(s_rx_buffers); i++) {
    Mcp2515RxBuffer *rx_buf = &s_rx_buffers[i];
    if (int_flags & rx_buf->int_flag) {
      // Read ID
      uint8_t id_payload[] = { MCP2515_CMD_READ_RX | rx_buf->id };
      Mcp2515IdRegs read_id_regs = { 0 };
      spi_exchange(s_storage->spi_port, id_payload, sizeof(id_payload), (uint8_t *)&read_id_regs,
                   sizeof(read_id_regs));

      Mcp2515Id id = {
        .sid_0_2 = read_id_regs.sidl.sid_0_2,
        .sidh = read_id_regs.sidh,
        .eid0 = read_id_regs.eid0,
        .eid8 = read_id_regs.eid8,
        .eid_16_17 = read_id_regs.sidl.eid_16_17,
      };

      rx_msg.extended = read_id_regs.sidl.ide;
      rx_msg.dlc = read_id_regs.dlc.dlc;

      if (!rx_msg.extended) {
        // Standard IDs have garbage in the extended fields
        id.raw >>= MCP2515_EXTENDED_ID_LEN;
      }

      rx_msg.id.raw = id.raw;

      uint8_t data_payload[] = { MCP2515_CMD_READ_RX | rx_buf->data };
      uint64_t read_data = 0;
      spi_exchange(s_storage->spi_port, data_payload, sizeof(data_payload), (uint8_t *)&rx_msg.data,
                   sizeof(rx_msg.data));
      // Clear the interrupt flag so a new message can be loaded
      prv_bit_modify(MCP2515_CTRL_REG_CANINTF, rx_buf->int_flag, 0x0);

      can_queue_push(&s_storage->rx_queue, &rx_msg);
    }
  }
}

static void prv_handle_error(uint8_t int_flags, uint8_t err_flags) {
  // Clear flags
  if (int_flags & MCP2515_CANINT_EFLAG) {
    // Clear error flag
    prv_bit_modify(MCP2515_CTRL_REG_CANINTF, MCP2515_CANINT_EFLAG, 0);
  }

  if (err_flags & (MCP2515_EFLG_RX0_OVERFLOW | MCP2515_EFLG_RX1_OVERFLOW)) {
    // RX overflow - clear error flags
    uint8_t clear = 0;
    prv_write(MCP2515_CTRL_REG_EFLG, &clear, 1);
  }

  s_storage->errors.eflg = err_flags;
  prv_read(MCP2515_CTRL_REG_TEC, &s_storage->errors.tec, 1);
  prv_read(MCP2515_CTRL_REG_REC, &s_storage->errors.rec, 1);

  if (err_flags) {
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

  // set RXB0 ctrl BUKT bit on to enable rollover to rx1
  prv_bit_modify(MCP2515_CTRL_REG_RXB0CTRL, 1 << 3, 1 << 3);
  // 5.7 Timing configurations:
  // In order:
  // CNF3: PS2 Length = 6
  // CNF2: PS1 Length = 8, PRSEG Length = 1
  // CNF1: BRP = 0 (500kbps), 1 (250kbps), 3 (125kbps)
  // CANINTE: Enable error and receive interrupts
  // CANINTF: clear all IRQ flags
  // EFLG: clear all error flags
  const uint8_t s_registers[] = {
    0x05,
    MCP2515_CNF2_BTLMODE_CNF3 | MCP2515_CNF2_SAMPLE_3X | (0x07 << 3),
    s_brp_lookup[mcp2515_bitrate],
    MCP2515_CANINT_EFLAG | MCP2515_CANINT_RX1IE | MCP2515_CANINT_RX0IE,
    0x00,
    0x00,
  };

  prv_write(MCP2515_CTRL_REG_CNF3, s_registers, SIZEOF_ARRAY(s_registers));

  // Sanity check: read register after first write
  // If new reg value corresponds to expected
  uint8_t reg_val = 0;
  prv_read(MCP2515_CTRL_REG_CNF3, &reg_val, 1);

  LOG_DEBUG("MCP2515 Init Status: %s\n",
            reg_val == 0x05 ? "Connection SUCCESSFUL\n" : "Connection UNSUCCESSFUL\n");

  // Leave config mode
  uint8_t opmode =
      (mcp2515_loopback ? MCP2515_CANCTRL_OPMODE_LOOPBACK : MCP2515_CANCTRL_OPMODE_NORMAL);
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL, MCP2515_CANCTRL_OPMODE_MASK, opmode);

  return STATUS_CODE_OK;
}

TASK(MCP2515_INTERRUPT, TASK_MIN_STACK_SIZE) {
  mcp2515_hw_init_after_schedular_start();

  uint32_t notification;
  while (true) {
    notify_wait(&notification, BLOCK_INDEFINITELY);
    // Read CANINTF and EFLG
    struct {
      uint8_t canintf;
      uint8_t eflg;
    } regs;
    prv_read(MCP2515_CTRL_REG_CANINTF, (uint8_t *)&regs, sizeof(regs));
    // Mask out flags we don't care about
    regs.canintf &= MCP2515_CANINT_EFLAG | MCP2515_CANINT_RX0IE | MCP2515_CANINT_RX1IE;

    // Either RX or error
    prv_handle_rx(regs.canintf);
    prv_handle_error(regs.canintf, regs.eflg);
  }
}

// Initializes CAN using the specified settings.
StatusCode mcp2515_hw_init(const CanQueue *rx_queue, const Mcp2515Settings *settings) {
  // settings->spi_settings.mode = SPI_MODE_0;
  status_ok_or_return(spi_init(settings->spi_port, &settings->spi_settings));

  if (settings->can_settings.bitrate > CAN_HW_BITRATE_500KBPS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "mcp2515 does not support this bitrate");
  }

  mcp2515_bitrate = settings->can_settings.bitrate;
  mcp2515_loopback = settings->can_settings.loopback;

  // active low
  status_ok_or_return(
      gpio_init_pin(&settings->interrupt_pin, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH));
  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .edge = INTERRUPT_EDGE_FALLING,
  };
  status_ok_or_return(
      gpio_it_register_interrupt(&settings->interrupt_pin, &it_settings, 0, MCP2515_INTERRUPT));

  // ! Ensure the task priority is higher than the rx/tx tasks in mcp2515.c
  status_ok_or_return(tasks_init_task(MCP2515_INTERRUPT, TASK_PRIORITY(3), NULL));

  return STATUS_CODE_OK;
}

StatusCode mcp2515_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended);

StatusCode mcp2515_hw_add_filter_out(uint32_t mask, uint32_t filter, bool extended);

// CanHwBusStatus mcp2515_hw_bus_status(void);

StatusCode mcp2515_hw_transmit(uint32_t id, bool extended, const uint64_t data, size_t len) {
  // Ensure the CANCTRL register is set to the correct value
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL, 0x1f, 0x0f);
  // Get free transmit buffer
  uint8_t tx_status = __builtin_ffs(
      ~prv_read_status() & (MCP2515_STATUS_TX0REQ | MCP2515_STATUS_TX1REQ | MCP2515_STATUS_TX2REQ));
  if (tx_status == 0) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  // Status format: 0b01010100 = all TXxREQ bits set
  // ffs returns 1-indexed: (x-3)/2 -> 0b00000111 = all TXxREQ bits set
  Mcp2515TxBuffer *tx_buf = &s_tx_buffers[(tx_status - 3) / 2];

  Mcp2515Id tx_id = { .raw = id };
  // If it's a standard id, make sure it lines up in the right bits
  if (tx_id.raw >> MCP2515_STANDARD_ID_LEN == 0) {
    tx_id.raw <<= MCP2515_EXTENDED_ID_LEN;
  }

  Mcp2515IdRegs tx_id_regs = {
    .sidh = tx_id.sidh,
    .sidl = { .eid_16_17 = tx_id.eid_16_17,
              .ide = extended,
              .srr = false,
              .sid_0_2 = tx_id.sid_0_2 },
    .eid0 = tx_id.eid0,
    .eid8 = tx_id.eid8,
    .dlc = { .dlc = len, .rtr = false },
  };

  // Load ID: SIDH, SIDL, EID8, EID0, RTSnDLC
  uint8_t id_payload[] = {
    MCP2515_CMD_LOAD_TX | tx_buf->id,
    tx_id_regs.sidh,
    tx_id_regs.sidl.raw,
    tx_id_regs.eid8,
    tx_id_regs.eid0,
    tx_id_regs.dlc.raw,
  };
  status_ok_or_return(spi_exchange(s_storage->spi_port, id_payload, sizeof(id_payload), NULL, 0));

  // Load data
  Mcp2515LoadTxPayload data_payload = {
    .cmd = MCP2515_CMD_LOAD_TX | tx_buf->data,
    .data = data,
  };
  status_ok_or_return(
      spi_exchange(s_storage->spi_port, (uint8_t *)&data_payload, sizeof(data_payload), NULL, 0));

  // Send message
  uint8_t send_payload[] = { MCP2515_CMD_RTS | tx_buf->rts };
  status_ok_or_return(
      spi_exchange(s_storage->spi_port, send_payload, sizeof(send_payload), NULL, 0));

  return STATUS_CODE_OK;
}

// Must be called within the RX handler, returns whether a message was processed
// bool mcp2515_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len);

// Call with MCP2515 in Config mode to set filters
static void prv_configure_filters(CanMessageId *filters) {
  Mcp2515Id default_filter = { .raw = filters[MCP2515_FILTER_ID_RXF0] };
  for (size_t i = 0; i < NUM_MCP2515_FILTER_IDS; i++) {
    Mcp2515Id filter = { .raw = filters[i] };
    if (default_filter.raw == 0) {
      continue;
    }

    // Prevents us from filtering for id 0x0
    if (filters[i] == 0) {
      filter = default_filter;
    }

    uint8_t maskRegH = MCP2515_REG_RXM0SIDH;
    if (i == MCP2515_FILTER_ID_RXF1) maskRegH = MCP2515_REG_RXM1SIDH;
    // If it's a standard id, ensure it's placed in the right bits
    if (filter.raw >> MCP2515_STANDARD_ID_LEN == 0) {
      filter.raw <<= MCP2515_EXTENDED_ID_LEN;
    }
    bool standard = filter.raw << (32 - MCP2515_EXTENDED_ID_LEN) == 0;
    size_t numMaskRegisters =
        standard ? MCP2515_NUM_MASK_REGISTERS_STANDARD : MCP2515_NUM_MASK_REGISTERS_EXTENDED;
    // Set the filter masks to 0xff so we filter on the whole message
    for (size_t i = 0; i < numMaskRegisters; i++) {
      prv_bit_modify(maskRegH + i, 0xff, 0xff);
    }
    // If it is just a standard id, then shift it up to match the struct
    uint8_t filterRegH = MCP2515_REG_RXF0SIDH;
    if (i == MCP2515_FILTER_ID_RXF1) filterRegH = MCP2515_REG_RXF1SIDH;
    uint8_t filterRegL = filterRegH + 1;
    // Set sidh
    prv_bit_modify(filterRegH, 0xff, filter.sidh);
    // Set sidl and eid16-17
    prv_bit_modify(filterRegL, 0xff, (filter.sid_0_2 << 5) | ((!standard) << 3) | filter.eid_16_17);
    // Set eid8-15
    prv_bit_modify(filterRegH + 2, 0xff, filter.eid8);
    // Set eid0-7
    prv_bit_modify(filterRegH + 3, 0xff, filter.eid0);
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
