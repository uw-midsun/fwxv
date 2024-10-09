#include "spi.h"

#include "log.h"
#include "queues.h"
#include "semaphore.h"
#include "spi_mcu.h"

#define SPI_BUF_SIZE 32

typedef struct {
  uint8_t tx_buf[SPI_BUF_SIZE];
  Queue tx_queue;
  uint8_t rx_buf[SPI_BUF_SIZE];
  Queue rx_queue;
  Mutex mutex;
} SPIBuffer;

typedef struct {
  GpioState cs_state;
  SPIBuffer spi_buf;
} SpiPortData;

static SpiPortData s_port[NUM_SPI_PORTS];

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  } else if (settings->mode >= NUM_SPI_MODES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI mode.");
  }

  s_port[spi].spi_buf.rx_queue.num_items = SPI_MAX_NUM_DATA;
  s_port[spi].spi_buf.rx_queue.item_size = sizeof(uint8_t);
  s_port[spi].spi_buf.rx_queue.storage_buf = s_port[spi].spi_buf.rx_buf;
  s_port[spi].spi_buf.tx_queue.num_items = SPI_MAX_NUM_DATA;
  s_port[spi].spi_buf.tx_queue.item_size = sizeof(uint8_t);
  s_port[spi].spi_buf.tx_queue.storage_buf = s_port[spi].spi_buf.tx_buf;
  status_ok_or_return(mutex_init(&s_port[spi].spi_buf.mutex));
  status_ok_or_return(queue_init(&s_port[spi].spi_buf.rx_queue));
  status_ok_or_return(queue_init(&s_port[spi].spi_buf.tx_queue));

  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_EMPTY, "Invalid SPI port.");
  }
  // Proceed if mutex is unlocked
  status_ok_or_return(mutex_lock(&s_port[spi].spi_buf.mutex, SPI_TIMEOUT_MS));

  queue_reset(&s_port[spi].spi_buf.tx_queue);
  queue_reset(&s_port[spi].spi_buf.rx_queue);

  // Copy data into queue
  uint8_t data = 0;
  for (size_t i = 0; i < tx_len + rx_len; i++) {
    data = (i < tx_len) ? tx_data[i] : 0;
    if (queue_send(&s_port[spi].spi_buf.tx_queue, &data, 0)) {
      mutex_unlock(&s_port[spi].spi_buf.mutex);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  // set spi CS state LOW
  s_port[spi].cs_state = GPIO_STATE_LOW;

  for (size_t i = 0; i < tx_len + rx_len; i++) {
    if (queue_receive(&s_port[spi].spi_buf.rx_queue, &data, SPI_TIMEOUT_MS)) {
      // timeout
      // set spi CS state HIGH
      s_port[spi].cs_state = GPIO_STATE_HIGH;
      mutex_unlock(&s_port[spi].spi_buf.mutex);
      return STATUS_CODE_TIMEOUT;
    }
    if (i >= tx_len) {
      // ignore first tx_len num of receieved data, store the rest into rx_data
      rx_data[i - tx_len] = data;
    }
  }

  // set spi CS state HIGH
  s_port[spi].cs_state = GPIO_STATE_HIGH;
  mutex_unlock(&s_port[spi].spi_buf.mutex);

  return STATUS_CODE_OK;
}

inline StatusCode spi_exchange_noreset(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) 
{
  GpioState prev_state = s_port[spi].cs_state;
  StatusCode s = spi_exchange(spi, tx_data, tx_len, rx_data, rx_len);
  s_port[spi].cs_state = prev_state;
  return s;

}

StatusCode spi_get_tx(SpiPort spi, uint8_t *data, uint8_t len) {
  uint8_t dummy = 0;
  for (size_t tx = 0; tx < len; tx++) {
    if (queue_receive(&s_port[spi].spi_buf.tx_queue, &data[tx], SPI_TIMEOUT_MS)) {
      queue_reset(&s_port[spi].spi_buf.tx_queue);
      return STATUS_CODE_EMPTY;
    }
    queue_send(&s_port[spi].spi_buf.rx_queue, &dummy, SPI_TIMEOUT_MS);
  }

  return STATUS_CODE_OK;
}

StatusCode spi_cs_set_state(SpiPort spi, GpioState state) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_EMPTY, "Invalid SPI port.");
  }
  s_port[spi].cs_state = state;
  return STATUS_CODE_OK;
}

StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len) {
  uint8_t dummy = 0;
  for (size_t rx = 0; rx < len; rx++) {
    if (queue_receive(&s_port[spi].spi_buf.tx_queue, &dummy, SPI_TIMEOUT_MS)) {
      queue_reset(&s_port[spi].spi_buf.rx_queue);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
    queue_send(&s_port[spi].spi_buf.rx_queue, &data[rx], SPI_TIMEOUT_MS);
  }

  return STATUS_CODE_OK;
}
