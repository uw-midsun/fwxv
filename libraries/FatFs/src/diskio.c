/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h" /* Declarations of disk functions */

#include <stdint.h>
#include <string.h>

#include "ff.h" /* Obtains integer types */
#include "gpio.h"
#include "spi.h"
#include "status.h"
#include "stm32_eval_spi_sd.h"

#define BAUDRATE 57600
#define SD_SECTOR_SIZE 512

#define NUM_BYTES_TO_READ 1
#define NUM_BYTES_TO_WRITE 1

GpioState card_detect_state;

DSTATUS disk_status(char pdrv /* Physical drive nmuber to identify the drive */
) {
  if (pdrv != DEV_MMC) {
    return STA_NOINIT;
  }
  GpioAddress card_detect_addr = { GPIO_PORT_A, 10 };
  gpio_get_state(&card_detect_addr, &card_detect_state);

  if (card_detect_state == GPIO_STATE_HIGH) {
    return STA_OK;
  } else {
    return STA_NODISK;
  }
}

DSTATUS disk_initialize(char pdrv) {
  if (pdrv != DEV_MMC) {
    return STA_NOINIT;
  }

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  SpiSettings spi_settings = { .baudrate = 57600,
                               .mode = SPI_MODE_0,
                               .mosi = { .port = GPIO_PORT_B, .pin = 15 },
                               .miso = { .port = GPIO_PORT_B, .pin = 14 },
                               .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                               .cs = { .port = GPIO_PORT_B, .pin = 12 } };

  spi_init(SPI_PORT_2, &spi_settings);

  GpioAddress card_detect_addr = { GPIO_PORT_A, 10 };
  gpio_init_pin(&card_detect_addr, GPIO_INPUT_PULL_DOWN, GPIO_STATE_HIGH);
  gpio_get_state(&card_detect_addr, &card_detect_state);

  if ((card_detect_state == GPIO_STATE_HIGH)) {
    SD_Init();
    return STA_OK;
  } else {
    return STA_NODISK;
  }
}

DRESULT disk_write(char pdrv,           /* Physical drive nmuber to identify the drive */
                   const uint8_t *buff, /* Data to be written */
                   uint32_t sector,     /* Start sector in LBA */
                   uint32_t count       /* Number of sectors to write */
) {
  if (pdrv != DEV_MMC) {
    return RES_PARERR;
  }

  for (uint32_t i = 0; i < count; i++) {
    if (spi_tx(SPI_PORT_2, buff, NUM_BYTES_TO_WRITE) != STATUS_CODE_OK) {
      return RES_ERROR;
    }
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(char pdrv,           /* Physical drive nmuber to identify the drive */
                  const uint8_t *buff, /* Data buffer to store read data */
                  uint32_t sector,     /* Start sector in LBA */
                  uint32_t count       /* Number of sectors to read */
) {
  if (pdrv != DEV_MMC) {
    return RES_PARERR;
  }

  for (uint32_t i = 0; i < count; i++) {
    if (spi_rx(SPI_PORT_2, buff, NUM_BYTES_TO_READ, SD_DUMMY_BYTE) != STATUS_CODE_OK) {
      return RES_ERROR;
    }
  }

  return RES_OK;
}

DRESULT disk_ioctl(char pdrv, char cmd, void *buff) {
  return RES_OK;
}