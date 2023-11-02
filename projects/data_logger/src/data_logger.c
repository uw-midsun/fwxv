#include "data_logger.h"

#include <stdint.h>
#include <string.h>

#include "diskio.h"
#include "ff.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"
#include "status.h"

#define BAUDRATE 57600
#define LOGICAL_DRIVE_NUMBER 0

#define FILE_DIRECTORY "log.txt"

#define MOUNT_IMMEDIATELY 1

FIL file_object;
FATFS fs;

DSTATUS data_logger_init() {
  DSTATUS init_status = disk_initialize(DEV_MMC);

  if (init_status != STA_OK) {
    return init_status;
  }

  return STA_OK;
}

FRESULT data_logger_mount() {
  FRESULT mount_status = f_mount(&fs, FILE_DIRECTORY, MOUNT_IMMEDIATELY);

  if (mount_status != FR_OK) {
    return mount_status;
  }

  return FR_OK;
}

FRESULT data_logger_write(const char *data) {
  FRESULT write_status;
  write_status = f_open(&file_object, FILE_DIRECTORY, FA_READ | FA_WRITE | FA_OPEN_APPEND);
  if (write_status != FR_OK) {
    return write_status;
  }

  unsigned int bytes_written = 0;
  write_status = f_write(&file_object, data, strlen(data), &bytes_written);
  if (write_status != FR_OK) {
    LOG_DEBUG("Failed to write to file: %d", write_status);
    return write_status;
  }

  if (bytes_written != strlen(data)) {
    LOG_DEBUG("Only %d bytes were written out of %d", bytes_written, strlen(data));
  }

  write_status = f_close(&file_object);
  if (write_status != FR_OK) {
    LOG_DEBUG("Failed to close file: %d", write_status);
    return write_status;
  }

  return FR_OK;
}

FRESULT data_logger_stop() {
  FRESULT stop_status;
  stop_status = f_unmount(FILE_DIRECTORY);

  if (stop_status != FR_OK) {
    LOG_DEBUG("Failed to unmount: %d", stop_status);
    return stop_status;
  }

  return FR_OK;
}
