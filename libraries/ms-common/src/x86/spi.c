#include "spi.h"

#include "log.h"
#include "queues.h"
#include "semaphore.h"
#include "spi_mcu.h"

#define SPI_BUF_SIZE 32
#define SPI_QUEUE_DELAY_MS 0

typedef struct {
  uint8_t tx_buf[SPI_BUF_SIZE];
  Queue tx_queue;
  uint8_t rx_buf[SPI_BUF_SIZE];
  Queue rx_queue;
  Mutex mutex;
} SPIBuffer;

static SPIBuffer s_buf[NUM_SPI_PORTS];

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  } else if (settings->mode >= NUM_SPI_MODES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI mode.");
  }

  s_buf[spi].tx_queue.num_items = SPI_BUF_SIZE;
  s_buf[spi].tx_queue.item_size = sizeof(uint8_t);
  s_buf[spi].tx_queue.storage_buf = s_buf[spi].tx_buf;
  s_buf[spi].rx_queue.num_items = SPI_BUF_SIZE;
  s_buf[spi].rx_queue.item_size = sizeof(uint8_t);
  s_buf[spi].rx_queue.storage_buf = s_buf[spi].rx_buf;

  status_ok_or_return(mutex_init(&s_buf[spi].mutex));
  status_ok_or_return(queue_init(&s_buf[spi].rx_queue));
  status_ok_or_return(queue_init(&s_buf[spi].tx_queue));

  return STATUS_CODE_OK;
}

StatusCode spi_tx(SpiPort spi, uint8_t *tx_data, size_t tx_len) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }

  status_ok_or_return(mutex_lock(&s_buf[spi].mutex, SPI_MUTEX_WAIT_MS));

  for (size_t tx = 0; tx < tx_len; tx++) {
    if (queue_send(&s_buf[spi].tx_queue, &tx_data[tx], SPI_QUEUE_DELAY_MS)) {
      queue_reset(&s_buf[spi].tx_queue);
      mutex_unlock(&s_buf[spi].mutex);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  mutex_unlock(&s_buf[spi].mutex);
  return STATUS_CODE_OK;
}

StatusCode spi_rx(SpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }

  status_ok_or_return(mutex_lock(&s_buf[spi].mutex, SPI_MUTEX_WAIT_MS));

  for (size_t rx = 0; rx < rx_len; rx++) {
    if (queue_receive(&s_buf[spi].rx_queue, &rx_data[rx], SPI_QUEUE_DELAY_MS)) {
      queue_reset(&s_buf[spi].rx_queue);
      mutex_unlock(&s_buf[spi].mutex);
      return STATUS_CODE_EMPTY;
    }
  }

  mutex_unlock(&s_buf[spi].mutex);
  return STATUS_CODE_OK;
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
  status_ok_or_return(mutex_lock(&s_buf[spi].mutex, SPI_MUTEX_WAIT_MS));

  for (size_t tx = 0; tx < len; tx++) {
    if (queue_receive(&s_buf[spi].tx_queue, &data[tx], SPI_QUEUE_DELAY_MS)) {
      queue_reset(&s_buf[spi].tx_queue);
      mutex_unlock(&s_buf[spi].mutex);
      return STATUS_CODE_EMPTY;
    }
  }

  mutex_unlock(&s_buf[spi].mutex);
  return STATUS_CODE_OK;
}

StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len) {
  status_ok_or_return(mutex_lock(&s_buf[spi].mutex, SPI_MUTEX_WAIT_MS));

  for (size_t rx = 0; rx < len; rx++) {
    if (queue_send(&s_buf[spi].rx_queue, &data[rx], SPI_QUEUE_DELAY_MS)) {
      queue_reset(&s_buf[spi].rx_queue);
      mutex_unlock(&s_buf[spi].mutex);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  mutex_unlock(&s_buf[spi].mutex);
  return STATUS_CODE_OK;
}
