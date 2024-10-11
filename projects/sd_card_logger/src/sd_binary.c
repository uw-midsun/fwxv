/**
 *******************************************************************************
 * @attention
 *
 * COPYRIGHT(c) 2016 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

#include "sd_binary.h"
#include <string.h>
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"

// The size of a command frame for the SD card
#define SD_SEND_SIZE 6

// The amount of times to send a dummy byte before and/or after raising the CS
// line
#define SD_DUMMY_COUNT_CONST 8

// Amount of times to retry when doing initialization
#define SD_NUM_RETRIES 100

// The default byte to send
#define SD_DUMMY_BYTE (0xFF)

// Maximum amount of relevant response bytes (if response is R1b, we keep on reading the )
#define SD_MAX_RESPONSE_DATA (5)

#define SD_R1_IDLE (0x1)
#define SD_R1_ERASE_RESET (0x2)
#define SD_R1_ILLEGAL_CMD (0x4)
#define SD_R1_CRC_ERROR (0x8)
#define SD_R1_ERASE_ERROR (0x10)
#define SD_R1_ADDRESS_ERROR (0x20)
#define SD_R1_PARAM_ERROR (0x40)

#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ (0xFE)
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE (0xFE)
#define SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE (0xFC)
#define SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE (0xFD)

#define SD_CMD_GO_IDLE_STATE (0)
#define SD_CMD_SEND_OP_COND (1)
#define SD_CMD_SEND_IF_COND (8)
#define SD_CMD_SEND_CSD (9)
#define SD_CMD_SEND_CID (10)
#define SD_CMD_STOP_TRANSMISSION (12)
#define SD_CMD_SET_BLOCKLEN (16)
#define SD_CMD_READ_SINGLE_BLOCK (17)
#define SD_CMD_READ_MULTIPLE_BLOCK (18)
#define SD_CMD_SET_WR_BLOCK_ERASE_COUNT (23)
#define SD_CMD_WRITE_BLOCK (24)
#define SD_CMD_WRITE_MULTIPLE_BLOCK (25)
#define SD_CMD_APP_SEND_OP_COND (41)
#define SD_CMD_APP_CMD (55)
#define SD_CMD_READ_OCR (58)

#define SD_DATA_OK (0x5)
#define SD_DATA_CRC_ERROR (0xB)
#define SD_DATA_WRITE_ERROR (0xD)
#define SD_DATA_MALFORMED (0xFF)

#define SD_OCR_V2728 (0x10000)
#define SD_OCR_V2829 (0x8000)
#define SD_OCR_V2930 (0x4000)
#define SD_OCR_V3031 (0x2000)
#define SD_OCR_V3132 (0x1000)
#define SD_OCR_V3233 (0x800)
#define SD_OCR_V3334 (0x400)
#define SD_OCR_V3435 (0x200)
#define SD_OCR_V3536 (0x100)
#define SD_OCR_CAP_TYPE (0x2)
#define SD_OCR_POWER_STATUS (0x1)
#define SD_OCR_ALL (0x1FF00)

#define SD_R7_CHECK_PAT (0xFF)
#define SD_R7_VAC_BITS (0xF00)

typedef enum {
  SD_RESPONSE_R1,
  SD_RESPONSE_R1B,
  SD_RESPONSE_R3,
  SD_RESPONSE_R7,
  SD_RESPONSE_UNKNOWN,
  NUM_SD_RESPONSES
} SdResponseType;

typedef struct SdResponse {
  SdResponseType type;
  uint8_t r1;
  uint32_t ocr_value;
} SdResponse;

typedef struct SpiPortSdConfig {
  uint32_t voltage_range;
  bool is_card;
  bool is_high_capacity;
  bool is_ready;
  bool is_sdv2;
} SpiPortSdConfig;

static SpiPortSdConfig SpiPortSdConfig_default = {
  .voltage_range = 0,
  .is_card = false,
  .is_high_capacity = false,
  .is_ready = false,
  .is_sdv2 = false
};

static SpiPortSdConfig sd_port[NUM_SPI_PORTS];

static StatusCode prv_write_read_byte(SpiPort spi, uint8_t* out, uint8_t byte) {
  return spi_exchange_noreset(spi, &byte, 1, out, 1);
}

static StatusCode prv_read_byte(SpiPort spi, uint8_t* out) {
  return prv_write_read_byte(spi, out, 0xFF);
}

static StatusCode prv_read_multi(SpiPort spi, uint8_t* base_addr, size_t amount) {
  for (size_t i = 0; i < amount; i++) {
    StatusCode s = prv_read_byte(spi, base_addr + i * sizeof(uint8_t));
    if (s != STATUS_CODE_OK) return s;
  }
  return STATUS_CODE_OK;
}

static StatusCode prv_wait_sd_response(SpiPort spi, uint8_t* out) {
  uint8_t timeout_bytes = 8;
  volatile uint8_t readvalue;
  StatusCode s = STATUS_CODE_OK;
  do {
    s = prv_read_byte(spi, &readvalue);
    timeout_bytes--;
  } while ((readvalue == SD_DUMMY_BYTE) && timeout_bytes && s == STATUS_CODE_OK);
  uint8_t read_bytes = 0;
  while (readvalue != SD_DUMMY_BYTE && s == STATUS_CODE_OK && read_bytes < SD_MAX_RESPONSE_DATA) {
    *(out + read_bytes * sizeof(uint8_t)) = readvalue;
    read_bytes++;
    s = prv_read_byte(spi, &readvalue);
  }
  if (!read_bytes) return STATUS_CODE_TIMEOUT;
  // Burn busy time
  while (readvalue == 0 && s == STATUS_CODE_OK) {
    s = prv_read_byte(spi, &readvalue);
  }
  return s;
}

static StatusCode prv_send_cmd(SpiPort spi, SdResponse* response_field, uint8_t cmd, uint32_t arg) {
  uint8_t frame[SD_SEND_SIZE];

  // Split the cmd parameter into 8 byte ints
  frame[0] = (cmd | 0x40);
  frame[1] = (uint8_t)(arg >> 24);
  frame[2] = (uint8_t)(arg >> 16);
  frame[3] = (uint8_t)(arg >> 8);
  frame[4] = (uint8_t)(arg);
  frame[5] = (uint8_t)0x95; // CRC for a CMD0. We can hardcode this since the CRC is ignored from then on anyways.

  spi_cs_set_state(spi, GPIO_STATE_LOW);

  spi_tx(spi, frame, SD_SEND_SIZE);

  uint8_t* res[5];
  StatusCode read_result = prv_wait_sd_response(spi, &res);
  if (read_result != STATUS_CODE_OK) return read_result;
  response_field->r1 = res[0];
  response_field->ocr_value = *((uint32_t*)(res + 1));
  switch(cmd) {
    case SD_CMD_GO_IDLE_STATE:
    case SD_CMD_SEND_OP_COND:
    case SD_CMD_APP_SEND_OP_COND:
    case SD_CMD_SEND_CSD:
    case SD_CMD_SEND_CID:
    case SD_CMD_SET_BLOCKLEN:
    case SD_CMD_READ_SINGLE_BLOCK:
    case SD_CMD_READ_MULTIPLE_BLOCK:
    case SD_CMD_SET_WR_BLOCK_ERASE_COUNT:
    case SD_CMD_WRITE_BLOCK:
    case SD_CMD_WRITE_MULTIPLE_BLOCK:
    case SD_CMD_APP_CMD:
      response_field->type = SD_RESPONSE_R1;
      break;
    case SD_CMD_STOP_TRANSMISSION:
      response_field->type = SD_RESPONSE_R1B;
      break;
    case SD_CMD_READ_OCR:
      response_field->type = SD_RESPONSE_R3;
      break;
    case SD_CMD_SEND_IF_COND:
      response_field->type = SD_RESPONSE_R7;
      break;
    default:
      response_field->type = SD_RESPONSE_UNKNOWN;
      break;
  }
}

static StatusCode prv_sd_get_data_response(SpiPort spi) {
  volatile uint8_t readvalue;
  uint16_t timeout = 8;
  StatusCode s = prv_read_byte(spi, &readvalue);
  while (readvalue == SD_DUMMY_BYTE && timeout && s == STATUS_CODE_OK) {
    timeout--;
    s = prv_read_byte(spi, &readvalue);
  }
  if (s != STATUS_CODE_OK) return s;
  if (!timeout) return STATUS_CODE_TIMEOUT;
  if (readvalue == SD_DATA_OK) {
    // Wait for IO line to return to 0xFF
    while (prv_read_byte(spi, NULL) != SD_DUMMY_BYTE) {}
    return STATUS_CODE_OK;
  }
  return STATUS_CODE_INTERNAL_ERROR;
}

static bool prv_check_sd_response_blank(SdResponse response) {
  return response.r1 == 0xFF && response.ocr_value == 0xFFFFFFFF;
}

static void prv_pulse_idle(SpiPort spi) {
  spi_cs_set_state(spi, GPIO_STATE_HIGH);
  prv_read_byte(spi, NULL);
}

StatusCode sd_card_init(SpiPort spi) {
  volatile StatusCode last_status;
  volatile SdResponse last_response;
  volatile uint16_t retry_counter = SD_NUM_RETRIES;

  sd_port[spi] = SpiPortSdConfig_default;

  // Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and
  // wait for In Idle State Response (R1 Format) equal to 0x01
  do {
    last_status = prv_send_cmd(spi, &last_response, SD_CMD_GO_IDLE_STATE, 0);
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    if (!(--retry_counter)) {
      return status_msg(STATUS_CODE_TIMEOUT, "Fail to init SD card before timeout\n");
    }
    delay_ms(20);
  } while (!((last_response.r1 & SD_R1_IDLE) && last_status == STATUS_CODE_OK));

  retry_counter = SD_NUM_RETRIES;
  // Send CMD8 (SD_CMD_SEND_IF_COND) to check the power supply status. SDV2 only.
  // and wait until response (R7 Format). If there isn't a response after a set timeout, we'll stop the initialization.
  // Arg: Voltage Supplied is 2.7-3.6V, Check Pattern is 0xAA
  do {
    last_status = prv_send_cmd(spi, &last_response, SD_CMD_SEND_IF_COND, 0x1AA);
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    if (last_response.r1 & SD_R1_ILLEGAL_CMD) { // Legacy SD Card
      sd_port[spi].is_sdv2 = false;
      break;
    }
    if (!(--retry_counter)) {
      return status_msg(STATUS_CODE_TIMEOUT, "SD Card failed to give valid infrastructure condition in alloted number of retries\n");
    }
    if (!(last_response.ocr_value & SD_R7_VAC_BITS)) {
      return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD Card does not support provided voltage range\n");
    }
    if (last_response.ocr_value & SD_R7_CHECK_PAT == 0xAA) {
      sd_port[spi].is_sdv2 = true;
      break;
    }
  } while (true);

  // Check voltage again (SD1/SD2)
  last_status = prv_send_cmd(spi, &last_response, SD_CMD_READ_OCR, 0);
  spi_cs_set_state(spi, GPIO_STATE_HIGH); // Reset

  if (!(--retry_counter)) 
    return status_msg(STATUS_CODE_TIMEOUT, "SD Card failed to power within alloted timeframe\n");

  if (last_response.r1 & SD_R1_ILLEGAL_CMD) 
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Attached device does not recognize CMD58, aborting\n");
  uint32_t acceptable_v_range = SD_OCR_ALL;
  if (!(last_response.r1 & acceptable_v_range)) 
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD Card not in supported voltage range\n");
  if (sd_port[spi].is_sdv2)
    sd_port[spi].is_high_capacity = last_response.ocr_value & SD_OCR_CAP_TYPE;
  
  // Start initialization
  do {
    // Send ACMD41 (SD_CMD_SD_APP_OP_COND)
    // Arg: 0x40000000, SDHC/SDXC Support with Power Saving on

    last_status = prv_send_cmd(spi, &last_response, SD_CMD_APP_CMD, 0);
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    prv_read_byte(spi, NULL);

    last_status = prv_send_cmd(spi, &last_response, SD_CMD_APP_SEND_OP_COND, 0x40000000);
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    prv_read_byte(spi, NULL);
  } while (last_response.r1 & SD_R1_IDLE);
  
  return STATUS_CODE_OK;
}

StatusCode sd_wait_data(SpiPort spi, uint8_t data) {
  uint16_t timeout = 0xFFF;
  uint8_t readvalue;

  // Check if response matches data

  do {
    readvalue = prv_read_byte(spi);
    timeout--;
  } while ((readvalue != data) && timeout);

  if (timeout == 0) {
    // After time out
    return status_msg(STATUS_CODE_TIMEOUT, "Timed out while waiting\n");
  }

  return STATUS_CODE_OK;
}

StatusCode sd_read_blocks(SpiPort spi, uint8_t *dest, uint32_t ReadAddr, uint32_t NumberOfBlocks) {
  uint32_t offset = 0;
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response
  // (0x00: no errors)
  response = prv_send_cmd(spi, SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_pulse_idle(spi);

  if (response.data != SD_R1_NO_ERROR) {
    prv_pulse_idle(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR,
                      "Failed to read because SD card responded with an error\n");
  }

  // Data transfer
  while (NumberOfBlocks--) {
    // Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block
    // Check if the SD acknowledged the read block command: R1 response (0x00:
    // no errors)
    response = prv_send_cmd(spi, SD_CMD_READ_SINGLE_BLOCK, (ReadAddr + offset) / SD_BLOCK_SIZE,
                            0xFF, SD_RESPONSE_R1);
    if (response.r1 != SD_R1_NO_ERROR) {
      prv_pulse_idle(spi);
      return status_msg(STATUS_CODE_INTERNAL_ERROR,
                        "Failed to read because SD card responded with an error\n");
    }

    // Now look for the data token to signify the start of the data
    if (status_ok(sd_wait_data(spi, SD_TOKEN_START_DATA_SINGLE_BLOCK_READ))) {
      // Read the SD block data : read 512 bytes of data
      spi_exchange(spi, NULL, 0, dest + offset, SD_BLOCK_SIZE);
      // Set next read address
      offset += SD_BLOCK_SIZE;
      // get CRC bytes (not really needed by us, but required by SD)
      prv_write_dummy(spi, 3);
    } else {
      prv_pulse_idle(spi);
      return status_msg(STATUS_CODE_TIMEOUT, "SD card timeout\n");
    }

    // Sets the CS line to high to end the read transaction
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    prv_write_read_byte(spi, SD_DUMMY_BYTE);
  }

  prv_pulse_idle(spi);
  return STATUS_CODE_OK;
}

static StatusCode prv_sd_write_block(SpiPort spi, uint8_t *src, uint32_t WriteAddr) {
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response
  // (0x00: no errors)
  prv_send_cmd(spi, &response, SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE);
  prv_pulse_idle(spi);
  if (response.data != SD_R1_NO_ERROR) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card error\n");
  }

  // Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
  // Check if the SD acknowledged the write block command: R1 response (0x00: no
  // errors)

  prv_send_cmd(spi, &response, SD_CMD_WRITE_BLOCK, WriteAddr / SD_BLOCK_SIZE);
  if (response.data != SD_R1_NO_ERROR) {
    prv_pulse_idle(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card error\n");
  }

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  // Send the data token to signify the start of the data
  uint8_t dat = SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE;
  spi_tx(spi, &dat, 1);

  // Write the block data to SD
  spi_tx(spi, src, SD_BLOCK_SIZE);

  // Put CRC bytes (not really needed by us, but required by SD)
  uint16_t crc = crc15_calculate(src, SD_BLOCK_SIZE);
  spi_tx(spi, &crc, 2);

  if (!status_ok(prv_sd_get_data_response(spi))) {
    // Quit and return failed status
    prv_pulse_idle(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card error\n");
  }

  prv_pulse_idle(spi);
  return STATUS_CODE_OK;
}

// potentially autocalculate number of blocks?
StatusCode sd_write_blocks(SpiPort spi, uint8_t *src, uint32_t WriteAddr, uint32_t NumberOfBlocks) {
  if (!NumberOfBlocks) {
    return prv_sd_write_block(spi, src, WriteAddr);
  }

  uint32_t offset = 0;
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response
  // (0x00: no errors)
  prv_send_cmd(spi, &response, SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE);
  prv_pulse_idle(spi);
  if (response.data != SD_R1_NO_ERROR) {
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  // Data transfer
  prv_send_cmd(spi, &response, SD_CMD_WRITE_MULTIPLE_BLOCK, WriteAddr / SD_BLOCK_SIZE);
  if (response.data != SD_R1_NO_ERROR) {
    prv_pulse_idle(spi);
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  while (NumberOfBlocks--) {
    // Send the data token to signify the start of the data
    uint8_t dat = SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE;
    spi_tx(spi, &dat, 1);

    // Write the block data to SD
    spi_tx(spi, src + offset, SD_BLOCK_SIZE);

    // Set next write address
    offset += SD_BLOCK_SIZE;

    // Put CRC bytes (not really needed by us, but required by SD)
    uint16_t crc = crc15_calculate(src + offset, SD_BLOCK_SIZE);
    spi_tx(spi, &crc, 2);

    if (!status_ok(prv_sd_get_data_response(spi))) {
      // Quit and return failed status
      prv_pulse_idle(spi);
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
  }

  // Write the block data to SD
  uint8_t end_transmission = SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE;
  spi_tx(spi, &end_transmission, 1);

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  // Catch the last busy response
  volatile uint8_t dataresponse;
  uint16_t timeout = 0xFFFF;
  while ((dataresponse = prv_read_byte(spi)) == 0x00 && timeout) {
    timeout--;
  }

  prv_pulse_idle(spi);
  return STATUS_CODE_OK;
}

StatusCode sd_is_initialized(SpiPort spi) {
  SdResponse res = prv_send_cmd(spi, SD_CMD_STATUS, 0, SD_DUMMY_BYTE, SD_RESPONSE_R2);
  prv_write_dummy(spi, 1);

  if (res.r1 == 0 && res.r2 == 0) {
    return STATUS_CODE_OK;
  }

  return status_code(STATUS_CODE_INTERNAL_ERROR);
}