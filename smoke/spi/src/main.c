#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"
#include "tasks.h"

// ==== WRITE PARAMETERS ====

// Set this to true to perform an SPI write
#define SHOULD_WRITE true

// Fill in these variables with the port and address to write to.
#define WRITE_SPI_PORT SPI_PORT_1  // Could be SPI_Port_2

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0xFC };

// ==== READ PARAMETERS ====

// Set this to true to perform an SPI read.
#define SHOULD_READ true

// Fill in these variables with the port and address to read from.
#define READ_SPI_PORT SPI_PORT_1

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 1

// SPI commands: Table 12-1
#define MCP2515_CMD_RESET 0xC0
#define MCP2515_CMD_READ 0x03
#define MCP2515_CMD_READ_RX 0x90
#define MCP2515_CMD_WRITE 0x02
#define MCP2515_CMD_LOAD_TX 0x40
#define MCP2515_CMD_RTS 0x80
#define MCP2515_CMD_READ_STATUS 0xA0
#define MCP2515_CMD_RX_STATUS 0xB0
#define MCP2515_CMD_BIT_MODIFY 0x05

// READ_RX Arguments: Figure 12-3
#define MCP2515_READ_RXB0SIDH 0x00  // ID
#define MCP2515_READ_RXB0D0 0x02    // Data
#define MCP2515_READ_RXB1SIDH 0x04
#define MCP2515_READ_RXB1D0 0x06

// LOAD_TX Arguments: Figure 12-5
#define MCP2515_LOAD_TXB0SIDH 0x00
#define MCP2515_LOAD_TXB0D0 0x01
#define MCP2515_LOAD_TXB1SIDH 0x02
#define MCP2515_LOAD_TXB1D0 0x03
#define MCP2515_LOAD_TXB2SIDH 0x04
#define MCP2515_LOAD_TXB2D0 0x05

// RTS Arguments: Table 12-1 (RTS)
#define MCP2515_RTS_TXB0 0x01
#define MCP2515_RTS_TXB1 0x02
#define MCP2515_RTS_TXB2 0x04

// READ_STATUS Response: Figure 12-8
#define MCP2515_STATUS_RX0IF 0x01
#define MCP2515_STATUS_RX1IF 0x02
#define MCP2515_STATUS_TX0REQ 0x04
#define MCP2515_STATUS_TX0IF 0x08
#define MCP2515_STATUS_TX1REQ 0x10
#define MCP2515_STATUS_TX1IF 0x20
#define MCP2515_STATUS_TX2REQ 0x40
#define MCP2515_STATUS_TX2IF 0x80

// Control Registers: Table 11-2
#define MCP2515_CTRL_REG_BFPCTRL 0x0C    // RX pins disabled by default
#define MCP2515_CTRL_REG_TXRTSCTRL 0x0D  // TX pins input by default
#define MCP2515_CTRL_REG_CANSTAT 0x0E
#define MCP2515_CTRL_REG_CANCTRL 0x0F
#define MCP2515_CTRL_REG_TEC 0x1C
#define MCP2515_CTRL_REG_REC 0x1D
#define MCP2515_CTRL_REG_CNF3 0x28
#define MCP2515_CTRL_REG_CNF2 0x29
#define MCP2515_CTRL_REG_CNF1 0x2A
#define MCP2515_CTRL_REG_CANINTE 0x2B
#define MCP2515_CTRL_REG_CANINTF 0x2C
#define MCP2515_CTRL_REG_EFLG 0x2D
#define MCP2515_CTRL_REG_TXB0CTRL 0x30
#define MCP2515_CTRL_REG_TXB1CTRL 0x40
#define MCP2515_CTRL_REG_TXB2CTRL 0x50
#define MCP2515_CTRL_REG_RXB0CTRL 0x60
#define MCP2515_CTRL_REG_RXB1CTRL 0x70

// TX Ctrl bits: Register 3-1
#define MCP2515_TX_CTRL_BIT_ABTF (1 << 6)
#define MCP2515_TX_CTRL_BIT_MLOA (1 << 5)
#define MCP2515_TX_CTRL_BIT_TXERR (1 << 4)

// Filter/Mask Registers: Table 11-1
#define MCP2515_REG_RXF0SIDH 0x00
#define MCP2515_REG_RXF1SIDH 0x04
#define MCP2515_REG_RXF2SIDH 0x08
#define MCP2515_REG_RXF3SIDH 0x10
#define MCP2515_REG_RXF4SIDH 0x14
#define MCP2515_REG_RXF5SIDH 0x18
#define MCP2515_REG_RXM0SIDH 0x20
#define MCP2515_REG_RXM1SIDH 0x24

// CANCTRL: Register 10-1
#define MCP2515_CANCTRL_OPMODE_MASK 0xE0
#define MCP2515_CANCTRL_OPMODE_NORMAL 0x00
#define MCP2515_CANCTRL_OPMODE_SLEEP 0x20
#define MCP2515_CANCTRL_OPMODE_LOOPBACK 0x40
#define MCP2515_CANCTRL_OPMODE_LISTEN 0x60
#define MCP2515_CANCTRL_OPMODE_CONFIG 0x80

#define MCP2515_CANCTRL_CLKOUT_MASK 0x07
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_1 0x04  // CLKEN is automatically enabled
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_2 0x05
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_4 0x06
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_8 0x07

// CNF3: Register 5-3
#define MCP2515_CNF3_PHSEG2_MASK 0x07

// CNF2: Register 5-2
#define MCP2515_CNF2_BTLMODE_MASK 0x80
#define MCP2515_CNF2_BTLMODE_CNF3 0x80

#define MCP2515_CNF2_SAMPLE_MASK 0x40
#define MCP2515_CNF2_SAMPLE_3X 0x40

#define MCP2515_CNF2_PHSEG1_MASK 0x38
#define MCP2515_CNF2_PRSEG_MASK 0x07

// CNF1: Register 5-1
#define MCP2515_CNF1_BRP_MASK 0x3F

// CANINTE/INTF: Register 7-1/2
#define MCP2515_CANINT_MSG_ERROR 0x80
#define MCP2515_CANINT_WAKEUP 0x40
#define MCP2515_CANINT_EFLAG 0x20
#define MCP2515_CANINT_TX2IE 0x10
#define MCP2515_CANINT_TX1IE 0x08
#define MCP2515_CANINT_TX0IE 0x04
#define MCP2515_CANINT_RX1IE 0x02
#define MCP2515_CANINT_RX0IE 0x01

// EFLG: Register 6-3
#define MCP2515_EFLG_RX1_OVERFLOW 0x80  // RX1 overflow
#define MCP2515_EFLG_RX0_OVERFLOW 0x40
#define MCP2515_EFLG_TX_BUS_OFF 0x20
#define MCP2515_EFLG_TX_EP_FLAG 0x10
#define MCP2515_EFLG_RX_EP_FLAG 0x08
#define MCP2515_EFLG_TX_WARNING 0x04
#define MCP2515_EFLG_RX_WARNING 0x02
#define MCP2515_EFLG_ERROR_WARNING 0x01

// TXBnDLC: Register 3-7
#define MCP2515_TXBNDLC_RTR_SHIFT 6
#define MCP2515_TXBNDLC_RTR_FRAME 0x40
#define MCP2515_TXBNDLC_DLC_MASK 0x0F

// RXB0CTRL: Register 4-1
#define MCP2515_RXB0CTRL_BUKT 0x4

#define MCP2515_MAX_WRITE_BUFFER_LEN 10
#define MCP2515_STANDARD_ID_LEN 11
#define MCP2515_EXTENDED_ID_LEN 18

#define MCP2515_CAN_BRP_125KBPS 3
#define MCP2515_CAN_BRP_250KBPS 1
#define MCP2515_CAN_BRP_500KBPS 0

#define MCP2515_NUM_MASK_REGISTERS_STANDARD 2
#define MCP2515_NUM_MASK_REGISTERS_EXTENDED 4

// BFPCTRL: Register 4-3
#define MCP2515_BFPCTRL_B1BFS 0x20
#define MCP2515_BFPCTRL_B2BFS 0x10
#define MCP2515_BFPCTRL_B1BFE 0x08
#define MCP2515_BFPCTRL_B2BFE 0x04
#define MCP2515_BFPCTRL_B1BFM 0x02
#define MCP2515_BFPCTRL_B2BFM 0x01

// ==== END OF PARAMETERS ====

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


static const Mcp2515TxBuffer s_tx_buffers[] = {
  { .id = MCP2515_LOAD_TXB0SIDH, .data = MCP2515_LOAD_TXB0D0, .rts = MCP2515_RTS_TXB0 },
  { .id = MCP2515_LOAD_TXB1SIDH, .data = MCP2515_LOAD_TXB1D0, .rts = MCP2515_RTS_TXB1 },
  { .id = MCP2515_LOAD_TXB2SIDH, .data = MCP2515_LOAD_TXB2D0, .rts = MCP2515_RTS_TXB2 },
};

static const Mcp2515RxBuffer s_rx_buffers[] = {
  { .id = MCP2515_READ_RXB0SIDH, .data = MCP2515_READ_RXB0D0, .int_flag = MCP2515_CANINT_RX0IE },
  { .id = MCP2515_READ_RXB1SIDH, .data = MCP2515_READ_RXB1D0, .int_flag = MCP2515_CANINT_RX1IE },
};

static SpiSettings spi_settings = {
  .baudrate = 10000000,  // 10 Mhz
  .mode = SPI_MODE_3,
  .mosi = { GPIO_PORT_B, 15 },
  .miso = { GPIO_PORT_B, 14 },
  .sclk = { GPIO_PORT_B, 13 },
  .cs = { GPIO_PORT_B, 12 },
};

// static uint8_t prv_read_status() {
//   uint8_t payload[] = { MCP2515_CMD_READ_STATUS };
//   uint8_t read_data = 0;
//   spi_exchange(SPI_PORT_2, payload, sizeof(payload), &read_data, sizeof(read_data));

//   return read_data;
// }
#include <string.h>

// SPI commands - See Table 12-1
static void prv_reset() {
  uint8_t payload[] = { MCP2515_CMD_RESET };
  spi_tx(SPI_PORT_2, payload, sizeof(payload));
}

static void prv_read(uint8_t addr, uint8_t *read_data, size_t read_len) {
  uint8_t payload[] = { MCP2515_CMD_READ, addr };
  spi_exchange(SPI_PORT_2, payload, sizeof(payload), read_data, read_len);
}

static void prv_write(uint8_t addr, uint8_t *write_data, size_t write_len) {
  uint8_t payload[MCP2515_MAX_WRITE_BUFFER_LEN];
  payload[0] = MCP2515_CMD_WRITE;
  payload[1] = addr;
  memcpy(&payload[2], write_data, write_len);

  spi_tx(SPI_PORT_2, payload, write_len + 2);
}

// See 12.10: *addr = (data & mask) | (*addr & ~mask)
static void prv_bit_modify(uint8_t addr, uint8_t mask, uint8_t data) {
  uint8_t payload[] = { MCP2515_CMD_BIT_MODIFY, addr, mask, data };

  spi_tx(SPI_PORT_2, payload, sizeof(payload));
}

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
    MCP2515_CAN_BRP_500KBPS,
    MCP2515_CANINT_EFLAG,
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
  uint8_t opmode = false;
  prv_bit_modify(MCP2515_CTRL_REG_CANCTRL, MCP2515_CANCTRL_OPMODE_MASK, opmode);

  return STATUS_CODE_OK;
}

TASK(smoke_spi_task, TASK_STACK_512) {
  mcp2515_hw_init_after_schedular_start();
  while (true) {
    uint8_t reg_val = 0;
    prv_read(MCP2515_CTRL_REG_CNF3, &reg_val, 1);
    LOG_DEBUG("%x\n", reg_val);

    delay_ms(1000);
  }
}

int main() {
  gpio_init();
  tasks_init();
  log_init();
  // LOG_DEBUG("Welcome to TEST!");
  spi_init(SPI_PORT_2, &spi_settings);

  tasks_init_task(smoke_spi_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}
