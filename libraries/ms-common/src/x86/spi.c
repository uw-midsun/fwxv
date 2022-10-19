#include "spi.h"

#include "log.h"
#include "semaphore.h"
#include "spi_mcu.h"

#define SPI_BUF_SIZE 32

static Mutex s_spi_mutex;

typedef struct SpiBuffer {
  uint8_t tx_buffer[SPI_BUF_SIZE];
  uint8_t rx_buffer[SPI_BUF_SIZE];
} SpiBuffer;

static SpiBuffer s_buf[NUM_SPI_PORTS];

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  } else if (settings->mode >= NUM_SPI_MODES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI mode.");
  }

  status_ok_or_return(mutex_init(&s_spi_mutex));

  return STATUS_CODE_OK;
}

StatusCode spi_tx(SpiPort spi, uint8_t *tx_data, size_t tx_len) {
  if (mutex_lock(&s_spi_mutex, SPI_MUTEX_WAIT_MS) == STATUS_CODE_OK) {
    if (tx_len > SPI_BUF_SIZE) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid tx_len.");
    }

    for (size_t i = 0; i < tx_len; i++) {
      s_buf[spi].tx_buffer[i] = tx_data[i];
    }

    mutex_unlock(&s_spi_mutex);
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
}

StatusCode spi_rx(SpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  if (mutex_lock(&s_spi_mutex, SPI_MUTEX_WAIT_MS) == STATUS_CODE_OK) {
    if (rx_len > SPI_BUF_SIZE) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid rx_len.");
    }

    for (size_t i = 0; i < rx_len; i++) {
      rx_data[i] = s_buf[spi].rx_buffer[i];
    }

    mutex_unlock(&s_spi_mutex);
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
}

StatusCode spi_cs_set_state(SpiPort spi, GpioState state) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_cs_get_state(SpiPort spi, GpioState *input_state) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
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

StatusCode spi_get_tx(SpiPort spi, uint8_t *data, uint8_t len) {
  for (size_t i = 0; i < len; i++) {
    data[i] = s_buf[spi].tx_buffer[i];
  }
  return STATUS_CODE_OK;
}

StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len) {
  for (size_t i = 0; i < len; i++) {
    s_buf[spi].rx_buffer[i] = data[i];
  }
  return STATUS_CODE_OK;
}
