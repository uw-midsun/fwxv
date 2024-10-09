#pragma once
// Generic blocking SPI driver
// Requires GPIO to be initialized.
#include <stddef.h>

#include "gpio.h"
#include "spi_mcu.h"
#include "status.h"

#define SPI_TIMEOUT_MS 100
#define SPI_MAX_NUM_DATA 32

typedef enum {
  SPI_MODE_0 = 0,  // CPOL: 0 CPHA: 0
  SPI_MODE_1,      // CPOL: 0 CPHA: 1
  SPI_MODE_2,      // CPOL: 1 CPHA: 0
  SPI_MODE_3,      // CPOL: 1 CPHA: 1
  NUM_SPI_MODES,
} SpiMode;

typedef struct {
  uint32_t baudrate;
  SpiMode mode;

  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} SpiSettings;

// Note that our prescalers on STM32 must be a power of 2, so the actual
// baudrate may not be exactly as requested. Please verify that the actual
// baudrate is within bounds.
StatusCode spi_init(SpiPort spi, const SpiSettings *settings);

// This method will send |tx_len| bytes from |tx_data| to the spi port |spi|. It
// will not change the CS line state. The response bytes will be discarded.
#define spi_tx(spi, tx_data, tx_len) spi_exchange(spi, tx_data, tx_len, NULL, 0)

// This method will initiate a spi transaction on the |spi| port.
// First, it will pull CS low, then transmit |tx_len| number of bytes from |tx_data|,
// then receive |rx_len| number of bytes into |tx_data|, Finally pull CS high.
StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len);

StatusCode spi_exchange_noreset(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len);

StatusCode spi_cs_set_state(SpiPort spi, GpioState state);

#ifdef MS_PLATFORM_X86
// Reads data from the tx buffer into data
StatusCode spi_get_tx(SpiPort spi, uint8_t *data, uint8_t len);

// Writes from data into the rx buffer
StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len);
#endif
