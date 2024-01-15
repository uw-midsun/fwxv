#include <stdio.h>
#include "ltc_afe_impl.h"
#include "ltc6811.h"

#include "gpio.h"
#include "delay.h"
#include "log.h"
#include "spi.h"
#include "tasks.h"
#include "crc15.h"

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
    delay_ms(0.3);
}

// static StatusCode prv_build_cmd(uint16_t command, uint8_t *cmd, size_t len) {
//   if (len != LTC6811_CMD_SIZE) {
//     return status_code(STATUS_CODE_INVALID_ARGS);
//   }

//   cmd[0] = (uint8_t)(command >> 8);
//   cmd[1] = (uint8_t)(command & 0xFF);

//   uint16_t cmd_pec = crc15_calculate(cmd, 2);
//   cmd[2] = (uint8_t)(cmd_pec >> 8);
//   cmd[3] = (uint8_t)(cmd_pec);

//   return STATUS_CODE_OK;
// }


// static void print_cfg_reg(LtcAfeConfigRegisterData *data) {
//   LOG_DEBUG("CFG REG:\n\r");
//   LOG_DEBUG(" %d\n\r", data->adcopt);
//   LOG_DEBUG(" %d\n\r", data->swtrd);
//   LOG_DEBUG(" %d\n\r", data->refon);

//   LOG_DEBUG(" %d\n\r", data->gpio);  // GPIO pin control

//   LOG_DEBUG(" %d\n\r", data->undervoltage);  // Undervoltage Comparison Voltage
//   LOG_DEBUG(" %d\n\r", data->overvoltage);   // Overvoltage Comparison Voltage

//   LOG_DEBUG(" %d\n\r", data->discharge_bitset);
//   LOG_DEBUG(" %d\n\r", data->discharge_timeout);
// }

void printBytesInHex(unsigned char *array, size_t length) {
    for (size_t i = 0; i < length; i++) {
        LOG_DEBUG("%02X ", array[i]); // Print each byte in hexadecimal format
    }
    printf("\n\r");
}

TASK(smoke_spi_task, TASK_STACK_512) {
  spi_init(SPI_PORT_2, &spi_settings);
  // LtcAfeWriteConfigPacket config_packet = { 0 };
  while (true) {
    prv_wakeup_idle(&spi_settings);
    // uint8_t gpio_bits =
    //     LTC6811_GPIO1_PD_OFF | LTC6811_GPIO3_PD_OFF | LTC6811_GPIO4_PD_OFF | LTC6811_GPIO5_PD_OFF;
    // prv_build_cmd(LTC6811_WRCFG_RESERVED, config_packet.wrcfg, SIZEOF_ARRAY(config_packet.wrcfg));
    // uint8_t enable = gpio_bits;

    // uint16_t undervoltage = 0;
    // uint16_t overvoltage = 0;

    // config_packet.devices[0].reg.discharge_bitset = 0xA;
    // config_packet.devices[0].reg.discharge_timeout = LTC_AFE_DISCHARGE_TIMEOUT_30_S;

    // config_packet.devices[0].reg.adcopt = 1; // ((LTC_AFE_ADC_MODE_7KHZ + 1) > 3);
    // config_packet.devices[0].reg.swtrd = true;

    // config_packet.devices[0].reg.undervoltage = undervoltage;
    // config_packet.devices[0].reg.overvoltage = overvoltage;

    // // GPIO 1 is used to read data from the mux
    // config_packet.devices[0].reg.gpio = (enable >> 3);

    // uint16_t cfgr_pec = crc15_calculate((uint8_t *)&config_packet.devices[0].reg, 6);
    // config_packet.devices[0].pec = SWAP_UINT16(cfgr_pec);

    // size_t len = SIZEOF_LTC_AFE_WRITE_CONFIG_PACKET(1);

    // LOG_DEBUG("TX_LEN, data: %d\n\r", len);
    // print_cfg_reg(&config_packet.devices[0].reg);
    // printBytesInHex((unsigned char *)&config_packet, len);
    // StatusCode stat = spi_exchange(SPI_PORT_2, (uint8_t *)&config_packet, len, NULL, 0);

    
    // uint8_t rd_cmd[4] = { 0 };
    // prv_build_cmd(LTC6811_RDCFG_RESERVED, rd_cmd, 4);
    // //LtcAfeConfigRegisterData rd_data;

    // unsigned char rd_data[sizeof(LtcAfeConfigRegisterData) + 1];
    
    // StatusCode stat2 = spi_exchange(SPI_PORT_2, rd_cmd, 4, (uint8_t*)&rd_data, sizeof(LtcAfeConfigRegisterData));
    // LOG_DEBUG("STAT1 %d stat2: %d\n\r", stat, stat2);
    // printBytesInHex(rd_data, sizeof(LtcAfeConfigRegisterData));
    // //print_cfg_reg(&rd_data);
    // delay_ms(1000);

    static uint8_t tx_bytes[4] = { 0x00, 0x02, 0x2b, 0x0a };
    static uint8_t rx_bytes[6] = { 0 };
    StatusCode stat2 = spi_exchange(SPI_PORT_2, tx_bytes, 4, rx_bytes, 6);

    // static uint8_t tx_bytes[1] = { 0xaa };
    // static uint8_t rx_bytes[6] = { 0 };
    // StatusCode stat2 = spi_exchange(SPI_PORT_2, tx_bytes, 1, rx_bytes, 0);

    LOG_DEBUG("RECV: %x %x %x %x %x %x\n", rx_bytes[0], rx_bytes[1], rx_bytes[2], rx_bytes[3], rx_bytes[4], rx_bytes[5]);
    delay_ms(1000);
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
