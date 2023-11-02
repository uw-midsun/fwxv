#include <stdio.h>

#include "data_logger.h"
#include "delay.h"
#include "ff.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"
#include "status.h"
#include "stm32f10x_rcc.h"
#include "tasks.h"

static SpiSettings spi_settings = { .baudrate = 57600,
                                    .mode = SPI_MODE_0,
                                    .mosi = { .port = GPIO_PORT_B, .pin = 15 },
                                    .miso = { .port = GPIO_PORT_B, .pin = 14 },
                                    .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                                    .cs = { .port = GPIO_PORT_B, .pin = 12 } };

TASK(data_logger_task, TASK_STACK_512) {
  DSTATUS init_status = data_logger_init();
  if (init_status != STA_OK) {
    LOG_DEBUG("Failed to initialize the data logger, error code: %d\n", init_status);
  } else {
    LOG_DEBUG("Data logger initialized successfully.\n");
  }
  delay_ms(3000);

  FRESULT mount_status = data_logger_mount();
  if (mount_status != FR_OK) {
    LOG_DEBUG("Failed to mount the microSD, error code: %d\n", mount_status);
  } else {
    LOG_DEBUG("MicroSD mounted successfully.\n");
  }
  delay_ms(3000);

  const char *sample_data = "Hello";
  FRESULT write_status = data_logger_write(sample_data);
  if (write_status != FR_OK) {
    LOG_DEBUG("Failed to write data to the SD card, error code: %d\n", write_status);
  } else {
    LOG_DEBUG("Data written successfully.\n");
  }
  delay_ms(3000);

  FRESULT stop_status = data_logger_stop();
  if (stop_status != FR_OK) {
    LOG_DEBUG("Failed to unmount the SD card, error code: %d\n", stop_status);
  } else {
    LOG_DEBUG("Data Logger stopped successfully.\n");
  }
  delay_ms(3000);
}

int main(void) {
  tasks_init();
  data_logger_init();
  log_init();

  tasks_init_task(data_logger_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}
