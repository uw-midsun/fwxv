/**
 ******************************************************************************
 * @file    stm32_eval_spi_sd.c
 * @author  MCD Application Team
 * @version V4.5.0
 * @date    07-March-2011
 * @brief   This file provides a set of functions needed to manage the SPI SD
 *          Card memory mounted on STM32xx-EVAL board (refer to stm32_eval.h
 *          to know about the boards supporting this memory).
 *          It implements a high level communication layer for read and write
 *          from/to this memory. The needed STM32 hardware resources (SPI and
 *          GPIO) are defined in stm32xx_eval.h file, and the initialization is
 *          performed in SD_LowLevel_Init() function declared in stm32xx_eval.c
 *          file.
 *          You can easily tailor this driver to any other development board,
 *          by just adapting the defines for hardware resources and
 *          SD_LowLevel_Init() function.
 *
 *          +-------------------------------------------------------+
 *          |                     Pin assignment                    |
 *          +-------------------------+---------------+-------------+
 *          |  STM32 SPI Pins         |     SD        |    Pin      |
 *          +-------------------------+---------------+-------------+
 *          | SD_SPI_CS_PIN           |   ChipSelect  |    1        |
 *          | SD_SPI_MOSI_PIN / MOSI  |   DataIn      |    2        |
 *          |                         |   GND         |    3 (0 V)  |
 *          |                         |   VDD         |    4 (3.3 V)|
 *          | SD_SPI_SCK_PIN / SCLK   |   Clock       |    5        |
 *          |                         |   GND         |    6 (0 V)  |
 *          | SD_SPI_MISO_PIN / MISO  |   DataOut     |    7        |
 *          +-------------------------+---------------+-------------+
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32_eval_spi_sd.h"

#include "gpio.h"
#include "spi.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"

GpioAddress chip_select_addr = { GPIO_PORT_B, 12 };

/**
 * @brief  DeInitializes the SD/SD communication.
 * @param  None
 * @retval None
 */
void SD_DeInit(void) {
  return;
}

/**
 * @brief  Initializes the SD/SD communication.
 * @param  None
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
void SD_LowLevel_Init(void) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  static SpiSettings spi_settings = {
    .baudrate = 1000000,
    .mode = SPI_MODE_0,
    .mosi = { .port = GPIO_PORT_A, .pin = 7 },
    .miso = { .port = GPIO_PORT_A, .pin = 6 },
    .sclk = { .port = GPIO_PORT_A, .pin = 5 },
    .cs = { .port = GPIO_PORT_A, .pin = 4 },
  };

  SpiSettings *spi_init_settings = &spi_settings;

  spi_init(SPI_PORT_1, spi_init_settings);

  GpioAddress card_detect_addr = { GPIO_PORT_A, 10 };
  gpio_init_pin(&card_detect_addr, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);

  /*!< SD chip select high */
  gpio_set_state(&chip_select_addr, GPIO_STATE_HIGH);
}

void SD_LowLEvel_DeInit(void) {
  return;
}

SD_Error SD_Init(void) {
  uint32_t i = 0;

  /*!< Initialize SD_SPI */
  SD_LowLevel_Init();

  /*!< Send dummy byte 0xFF, 10 times with CS high */
  /*!< Rise CS and MOSI for 80 clocks cycles */
  for (i = 0; i <= 9; i++) {
    /*!< Send dummy byte 0xFF */
    SD_WriteByte(SD_DUMMY_BYTE);
  }
  /*------------Put SD in SPI mode--------------*/
  /*!< SD initialized and set to SPI mode properly */
  return (SD_GoIdleState());
}

/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
uint8_t SD_Detect(void) {
  __IO uint8_t status = SD_PRESENT;

  /*!< Check GPIO to detect SD */
  if (GPIO_ReadInputData(SD_DETECT_GPIO_PORT) & SD_DETECT_PIN) {
    status = SD_NOT_PRESENT;
  }
  return status;
}

/**
 * @brief  Reads a block of data from the SD.
 * @param  pBuffer: pointer to the buffer that receives the data read from the
 *                  SD.
 * @param  ReadAddr: SD's internal address to read from.
 * @param  BlockSize: the SD card Data block size.
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
SD_Error SD_ReadBlock(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t BlockSize) {
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  gpio_set_state(&chip_select_addr, GPIO_STATE_LOW);
  /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
  SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);

  /*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR)) {
    /*!< Now look for the data token to signify the start of the data */
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ)) {
      /*!< Read the SD block data : read NumByteToRead data */
      for (i = 0; i < BlockSize; i++) {
        /*!< Save the received data */
        *pBuffer = SD_ReadByte();

        /*!< Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /*!< Get CRC bytes (not really needed by us, but required by SD) */
      SD_ReadByte();
      SD_ReadByte();
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
  }
  /*!< SD chip select high */
  gpio_set_state(&chip_select_addr, GPIO_STATE_HIGH);

  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Returns the reponse */
  return rvalue;
}

/**
 * @brief  Reads multiple block of data from the SD.
 * @param  pBuffer: pointer to the buffer that receives the data read from the
 *                  SD.
 * @param  ReadAddr: SD's internal address to read from.
 * @param  BlockSize: the SD card Data block size.
 * @param  NumberOfBlocks: number of blocks to be read.
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
SD_Error SD_ReadMultiBlocks(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t BlockSize,
                            uint32_t NumberOfBlocks) {
  uint32_t i = 0, Offset = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  gpio_set_state(&chip_select_addr, GPIO_STATE_LOW);
  /*!< Data transfer */
  while (NumberOfBlocks--) {
    /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
    SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr + Offset, 0xFF);
    /*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
    if (SD_GetResponse(SD_RESPONSE_NO_ERROR)) {
      return SD_RESPONSE_FAILURE;
    }
    /*!< Now look for the data token to signify the start of the data */
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ)) {
      /*!< Read the SD block data : read NumByteToRead data */
      for (i = 0; i < BlockSize; i++) {
        /*!< Read the pointed data */
        *pBuffer = SD_ReadByte();
        /*!< Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /*!< Set next read address*/
      Offset += 512;
      /*!< get CRC bytes (not really needed by us, but required by SD) */
      SD_ReadByte();
      SD_ReadByte();
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    } else {
      /*!< Set response value to failure */
      rvalue = SD_RESPONSE_FAILURE;
    }
  }
  /*!< SD chip select high */
  gpio_set_state(&chip_select_addr, GPIO_STATE_HIGH);
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  /*!< Returns the reponse */
  return rvalue;
}

/**
 * @brief  Writes a block on the SD
 * @param  pBuffer: pointer to the buffer containing the data to be written on
 *                  the SD.
 * @param  WriteAddr: address to write on.
 * @param  BlockSize: the SD card Data block size.
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
SD_Error SD_WriteBlock(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t BlockSize) {
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  gpio_set_state(&chip_select_addr, GPIO_STATE_LOW);

  /*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write multiple block */
  SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0xFF);

  /*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR)) {
    /*!< Send a dummy byte */
    SD_WriteByte(SD_DUMMY_BYTE);

    /*!< Send the data token to signify the start of the data */
    SD_WriteByte(0xFE);

    /*!< Write the block data to SD : write count data by block */
    for (i = 0; i < BlockSize; i++) {
      /*!< Send the pointed byte */
      SD_WriteByte(*pBuffer);
      /*!< Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /*!< Put CRC bytes (not really needed by us, but required by SD) */
    SD_ReadByte();
    SD_ReadByte();

    /*!< Read data response */
    if (SD_GetDataResponse() == SD_DATA_OK) {
      rvalue = SD_RESPONSE_NO_ERROR;
    }
  }
  /*!< SD chip select high */
  gpio_set_state(&chip_select_addr, GPIO_STATE_HIGH);
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Returns the reponse */
  return rvalue;
}

/**
 * @brief  Writes many blocks on the SD
 * @param  pBuffer: pointer to the buffer containing the data to be written on
 *                  the SD.
 * @param  WriteAddr: address to write on.
 * @param  BlockSize: the SD card Data block size.
 * @param  NumberOfBlocks: number of blocks to be written.
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
SD_Error SD_WriteMultiBlocks(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t BlockSize,
                             uint32_t NumberOfBlocks) {
  uint32_t i = 0, Offset = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  gpio_set_state(&chip_select_addr, GPIO_STATE_LOW);
  /*!< Data transfer */
  while (NumberOfBlocks--) {
    /*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks */
    SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr + Offset, 0xFF);
    /*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
    if (SD_GetResponse(SD_RESPONSE_NO_ERROR)) {
      return SD_RESPONSE_FAILURE;
    }
    /*!< Send dummy byte */
    SD_WriteByte(SD_DUMMY_BYTE);
    /*!< Send the data token to signify the start of the data */
    SD_WriteByte(SD_START_DATA_SINGLE_BLOCK_WRITE);
    /*!< Write the block data to SD : write count data by block */
    for (i = 0; i < BlockSize; i++) {
      /*!< Send the pointed byte */
      SD_WriteByte(*pBuffer);
      /*!< Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /*!< Set next write address */
    Offset += 512;
    /*!< Put CRC bytes (not really needed by us, but required by SD) */
    SD_ReadByte();
    SD_ReadByte();
    /*!< Read data response */
    if (SD_GetDataResponse() == SD_DATA_OK) {
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    } else {
      /*!< Set response value to failure */
      rvalue = SD_RESPONSE_FAILURE;
    }
  }
  /*!< SD chip select high */
  gpio_set_state(&chip_select_addr, GPIO_STATE_HIGH);
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  /*!< Returns the reponse */
  return rvalue;
}

/**
 * @brief  Send 5 bytes command to the SD card.
 * @param  Cmd: The user expected command to send to SD card.
 * @param  Arg: The command argument.
 * @param  Crc: The CRC.
 * @retval None
 */
void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc) {
  uint32_t i = 0x00;

  uint8_t Frame[6];

  Frame[0] = (Cmd | 0x40); /*!< Construct byte 1 */

  Frame[1] = (uint8_t)(Arg >> 24); /*!< Construct byte 2 */

  Frame[2] = (uint8_t)(Arg >> 16); /*!< Construct byte 3 */

  Frame[3] = (uint8_t)(Arg >> 8); /*!< Construct byte 4 */

  Frame[4] = (uint8_t)(Arg); /*!< Construct byte 5 */

  Frame[5] = (Crc); /*!< Construct CRC: byte 6 */

  for (i = 0; i < 6; i++) {
    SD_WriteByte(Frame[i]); /*!< Send the Cmd bytes */
  }
}

/**
 * @brief  Get SD card data response.
 * @param  None
 * @retval The SD status: Read data response xxx0<status>1
 *         - status 010: Data accecpted
 *         - status 101: Data rejected due to a crc error
 *         - status 110: Data rejected due to a Write error.
 *         - status 111: Data rejected due to other error.
 */
uint8_t SD_GetDataResponse(void) {
  uint32_t i = 0;
  uint8_t response, rvalue;

  while (i <= 64) {
    /*!< Read resonse */
    response = SD_ReadByte();
    /*!< Mask unused bits */
    response &= 0x1F;
    switch (response) {
      case SD_DATA_OK: {
        rvalue = SD_DATA_OK;
        break;
      }
      case SD_DATA_CRC_ERROR:
        return SD_DATA_CRC_ERROR;
      case SD_DATA_WRITE_ERROR:
        return SD_DATA_WRITE_ERROR;
      default: {
        rvalue = SD_DATA_OTHER_ERROR;
        break;
      }
    }
    /*!< Exit loop in case of data ok */
    if (rvalue == SD_DATA_OK) break;
    /*!< Increment loop counter */
    i++;
  }

  /*!< Wait null data */
  while (SD_ReadByte() == 0)
    ;

  /*!< Return response */
  return response;
}

/**
 * @brief  Returns the SD response.
 * @param  None
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
SD_Error SD_GetResponse(uint8_t Response) {
  uint32_t Count = 0xFFF;

  /*!< Check if response is got or a timeout is happen */
  while ((SD_ReadByte() != Response) && Count) {
    Count--;
  }
  if (Count == 0) {
    /*!< After time out */
    return SD_RESPONSE_FAILURE;
  } else {
    /*!< Right response got */
    return SD_RESPONSE_NO_ERROR;
  }
}

/**
 * @brief  Put SD in Idle state.
 * @param  None
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
SD_Error SD_GoIdleState(void) {
  /*!< SD chip select low */
  gpio_set_state(&chip_select_addr, GPIO_STATE_LOW);

  /*!< Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode */
  SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95);

  /*!< Wait for In Idle State Response (R1 Format) equal to 0x01 */
  if (SD_GetResponse(SD_IN_IDLE_STATE)) {
    /*!< No Idle State Response: return response failue */
    return SD_RESPONSE_FAILURE;
  }
  /*----------Activates the card initialization process-----------*/
  do {
    /*!< SD chip select high */
    gpio_set_state(&chip_select_addr, GPIO_STATE_HIGH);

    /*!< Send Dummy byte 0xFF */
    SD_WriteByte(SD_DUMMY_BYTE);

    /*!< SD chip select low */
    gpio_set_state(&chip_select_addr, GPIO_STATE_LOW);

    /*!< Send CMD1 (Activates the card process) until response equal to 0x0 */
    // SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0xFF);
    /*!< Wait for no error Response (R1 Format) equal to 0x00 */
  } while (SD_GetResponse(SD_RESPONSE_NO_ERROR));
  /*!< SD chip select high */
  gpio_set_state(&chip_select_addr, GPIO_STATE_HIGH);

  /*!< Send dummy byte 0xFF */
  SD_WriteByte(SD_DUMMY_BYTE);

  return SD_RESPONSE_NO_ERROR;
}

/**
 * @brief  Write a byte on the SD.
 * @param  Data: byte to send.
 * @retval None
 */
uint8_t SD_WriteByte(uint8_t Data) {
  uint8_t read_bytes[1] = { 0 };
  uint8_t write_bytes[] = { Data };
  spi_exchange(SPI_PORT_1, write_bytes, SIZEOF_ARRAY(write_bytes), read_bytes, 1);

  return read_bytes[0];
}

/**
 * @brief  Read a byte from the SD.
 * @param  None
 * @retval The received byte.
 */
uint8_t SD_ReadByte(void) {
  uint8_t Data[1] = { 0 };
  static const uint8_t write_bytes[1] = { 0xFF };

  spi_exchange(SPI_PORT_1, write_bytes, 1, Data, 1);

  return Data[0];
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
