#pragma once
#include <stddef.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

#define SD_SPI_TIMEOUT_MS 100
#define SD_SPI_MAX_NUM_DATA 32

typedef enum {
  SD_SPI_PORT_1 = 0,
  SD_SPI_PORT_2,
  NUM_SD_SPI_PORTS,
} SdSpiPort;

typedef enum {
  SD_SPI_MODE_0 = 0,  // CPOL: 0 CPHA: 0
  SD_SPI_MODE_1,      // CPOL: 0 CPHA: 1
  SD_SPI_MODE_2,      // CPOL: 1 CPHA: 0
  SD_SPI_MODE_3,      // CPOL: 1 CPHA: 1
  NUM_SD_SPI_MODES,
} SdSpiMode;

typedef struct {
  uint32_t baudrate;
  SdSpiMode mode;

  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} SdSpiSettings;

// Note that our prescalers on STM32 must be a power of 2, so the actual baudrate may not be
// exactly as requested. Please verify that the actual baudrate is within bounds.
StatusCode sd_spi_init(SdSpiPort spi, SdSpiSettings *settings);

// This method will send |tx_len| bytes from |tx_data| to the spi port |spi|. It will not
// change the CS line state. The response bytes will be discarded.
StatusCode sd_spi_tx(SdSpiPort spi, uint8_t *tx_data, size_t tx_len);

// This method will receive |rx_len| bytes and place it into |rx_data| from the spi port
// |spi|. It will not change the CS line state. In order to receive data this method will
// send the byte specified by the |placeholder| parameter.
StatusCode sd_spi_rx(SdSpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder);

// This method will set the state of the CS line for a given spi port
StatusCode sd_spi_cs_set_state(SdSpiPort spi, GpioState state);

// Gets the CS state of a given spi port and assigns it to the state that is passed in.
StatusCode sd_spi_cs_get_state(SdSpiPort spi, GpioState *input_state);

StatusCode sd_spi_set_frequency(SdSpiPort spi, uint32_t baudrate);
