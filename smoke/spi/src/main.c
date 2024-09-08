#include <stdio.h>

#include "crc15.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "ltc_afe_impl.h"
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

// ==== END OF PARAMETERS ====

static SpiSettings spi_settings = {
  .baudrate = 100000,  // TO FIND
  .mode = SPI_MODE_3,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

static void prv_wakeup_idle(SpiSettings *settings) {
  // Wakeup method 2 - pair of long -1, +1 for each device
  gpio_set_state(&settings->cs, GPIO_STATE_LOW);
  gpio_set_state(&settings->cs, GPIO_STATE_HIGH);
  // Wait for 300us - greater than tWAKE, less than tIDLE
  delay_ms(1);
}

static StatusCode prv_build_cmd(uint16_t command, uint8_t *cmd) {
  cmd[0] = (uint8_t)(command >> 8);
  cmd[1] = (uint8_t)(command & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  LOG_DEBUG("CMD PEC: 0x%X\n", cmd_pec);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);

  return STATUS_CODE_OK;
}

void printBytesInHex(unsigned char *array, size_t length) {
  for (size_t i = 0; i < length; i++) {
    LOG_DEBUG("%02X \n\r", array[i]);  // Print each byte in hexadecimal format
  }
  printf("\n\r");
}

TASK(smoke_spi_task, TASK_STACK_512) {
  spi_init(SPI_PORT_2, &spi_settings);
  crc15_init_table();
  while (true) {
    // LTC6811
    uint8_t cmd[4];

    prv_build_cmd(0x01, cmd);

    // Wake
    prv_wakeup_idle(&spi_settings);
    static uint8_t rx_bytes[8] = { 0 };
    static uint8_t tx_bytes[12] = { 0x00, 0x01, 0x3d, 0x6e, 0xea, 0x00, 0x00, 0x00, 0x02, 0x04, 0x3F, 0x82 };
    static uint8_t pec_calc[6] = {  0xea, 0x00, 0x00, 0x00, 0x02, 0x04 };
    LOG_DEBUG("CRC15 FOR DATA CALCULATED: %d\n", crc15_calculate(pec_calc, 6));

    // Write Config
    spi_exchange(SPI_PORT_2, tx_bytes, 12, rx_bytes, 0);
    LOG_DEBUG("Wrote Config \n");

    // Idle
    delay_ms(10);
    prv_build_cmd(0x02, cmd);

    // Wake
    prv_wakeup_idle(&spi_settings);
    // Read Config
    static uint8_t tx_bytes2[4] = { 0x00, 0x02, 0x2b, 0x0a };
    spi_exchange(SPI_PORT_2, tx_bytes2, 4, rx_bytes, 8);
    LOG_DEBUG("Config Read: %x %x %x %x %x %x %x %x\n", rx_bytes[0], rx_bytes[1], rx_bytes[2],
              rx_bytes[3], rx_bytes[4], rx_bytes[5], rx_bytes[6], rx_bytes[7]);

    // Idle
    delay_ms(10);

    // Wake
    prv_wakeup_idle(&spi_settings);
    // Start conversion
    // 0b 00000 01 10 11 0 0 000
    // 0b 0000 0011 0110 0000
    static uint8_t tx_bytes3[4] = { 0b00000011, 0b01100000, 0xf4, 0x6c };
    spi_exchange(SPI_PORT_2, tx_bytes3, 4, rx_bytes, 0);

    // Wait for conversion to complete
    delay_ms(10);

    // Wake
    prv_wakeup_idle(&spi_settings);
    // Read converted data
    // 0b 00000 0 0 0 0 0 0 0 0 1 0 0
    // 0b 0000 0000 0000 0100
    static uint8_t tx_bytes4[4] = { 0b00000000, 0b00000100, 0x07, 0xc2 };
    spi_exchange(SPI_PORT_2, tx_bytes4, 4, rx_bytes, 8);
    LOG_DEBUG("Cell Voltage Group A: %x %x %x %x %x %x %x %x\n", rx_bytes[0], rx_bytes[1],
              rx_bytes[2], rx_bytes[3], rx_bytes[4], rx_bytes[5], rx_bytes[6], rx_bytes[7]);

    // Convert cell 1 to uV and mV
    uint32_t cell = 0;
    cell = (uint32_t)((rx_bytes[1] << 8) | rx_bytes[0]);
    LOG_DEBUG("Cell 1 raw hex: %lX\n", cell);
    cell *= 100;
    LOG_DEBUG("Cell 1 uV: %ld\n", cell);
    cell /= 1000;
    LOG_DEBUG("Cell 1 mV: %ld\n", cell);

    // IC fully asleep
    delay_ms(2000);
  }
}

int main() {
  gpio_init();
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(smoke_spi_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}