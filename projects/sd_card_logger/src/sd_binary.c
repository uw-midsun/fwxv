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

#include "crc15.h"
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
#define SD_R1_NO_ERRORS (0x7B)

#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ (0xFE)
#define SD_TOKEN_START_DATA_MULTI_BLOCK_READ (0xFE)
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE (0xFE)
#define SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE (0xFC)
#define SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE (0xFD)
#define SD_TOKEN_RESPONSE_MASK (0b00010001)
#define SD_TOKEN_RESPONSE_MASKED_VAL (1)
#define SD_TOKEN_RESPONSE_DATA_MASK (0b00001110)
#define SD_TOKEN_RESPONSE_ACCEPT (0b100)
#define SD_TOKEN_RESPONSE_CRC_ERROR (0b1010)
#define SD_TOKEN_RESPONSE_WRITE_ERROR (0b1100)
#define SD_TOKEN_ERRORLESS_MASK (0xF0)
#define SD_TOKEN_ERROR_GENERIC (0x1)
#define SD_TOKEN_ERROR_CC (0x2)
#define SD_TOKEN_ERROR_CARD_ECC (0x4)
#define SD_TOKEN_ERROR_OUT_OF_RANGE (0x8)

#define SD_CMD_GO_IDLE_STATE (0)
#define SD_CMD_SEND_OP_COND (1)
#define SD_CMD_SEND_IF_COND (8)
#define SD_CMD_SEND_CSD (9)
#define SD_CMD_SEND_CID (10)
#define SD_CMD_STOP_TRANSMISSION (12)
#define SD_CMD_SEND_STATUS (13)
#define SD_CMD_SET_BLOCKLEN (16)
#define SD_CMD_READ_SINGLE_BLOCK (17)
#define SD_CMD_READ_MULTIPLE_BLOCK (18)
#define SD_CMD_SET_WR_BLOCK_ERASE_COUNT (23)
#define SD_CMD_WRITE_BLOCK (24)
#define SD_CMD_WRITE_MULTIPLE_BLOCK (25)
#define SD_CMD_APP_SEND_OP_COND (41)
#define SD_CMD_APP_CMD (55)
#define SD_CMD_READ_OCR (58)

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

static SpiPortSdConfig SpiPortSdConfig_default = { .voltage_range = 0,
                                                   .is_card = false,
                                                   .is_high_capacity = false,
                                                   .is_ready = false,
                                                   .is_sdv2 = false };

static SpiPortSdConfig sd_port[NUM_SPI_PORTS];

static StatusCode prv_write_read_byte(SpiPort spi, uint8_t *out, uint8_t byte) {
  return spi_exchange_noreset(spi, &byte, 1, out, 1);
}

static StatusCode prv_read_byte(SpiPort spi, uint8_t *out) {
  return prv_write_read_byte(spi, out, 0xFF);
}

static StatusCode prv_read_multi(SpiPort spi, uint8_t *base_addr, size_t amount) {
  for (size_t i = 0; i < amount; i++) {
    StatusCode s = prv_read_byte(spi, base_addr + i * sizeof(uint8_t));
    if (s != STATUS_CODE_OK) return s;
  }
  return STATUS_CODE_OK;
}

static StatusCode prv_wait_sd_response(SpiPort spi, uint8_t *out) {
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

static void prv_end_transaction(SpiPort spi) {
  spi_cs_set_state(spi, GPIO_STATE_HIGH);
  prv_read_byte(spi, NULL);
}

static StatusCode prv_sd_get_next_data_token(SpiPort spi, uint8_t *token) {
  volatile StatusCode last_status;
  volatile uint8_t readvalue;
  uint16_t timeout = 0xFFF;

  do {
    last_status = prv_read_byte(spi, &readvalue);
    if (last_status != STATUS_CODE_OK) return last_status;
    if (!(timeout--))
      return status_msg(STATUS_CODE_TIMEOUT, "Timed out while waiting for data token\n");
  } while (readvalue == SD_DUMMY_BYTE);

  *token = readvalue;

  return STATUS_CODE_OK;
}

static StatusCode prv_sd_get_next_write_data_response(SpiPort spi, uint8_t *response) {
  volatile uint8_t readvalue;
  uint16_t timeout = 8;
  StatusCode s;

  do {
    s = prv_read_byte(spi, &readvalue);
    if (s != STATUS_CODE_OK) return s;
    if (--timeout)
      return status_msg(
          STATUS_CODE_TIMEOUT,
          "Timed out waiting for the data response after a write. This should not happen\n");
  } while (readvalue == SD_DUMMY_BYTE);

  if ((readvalue & SD_TOKEN_RESPONSE_MASK) == SD_TOKEN_RESPONSE_MASKED_VAL) {
    readvalue &= SD_TOKEN_RESPONSE_DATA_MASK;
    switch (readvalue) {
      case SD_TOKEN_RESPONSE_CRC_ERROR:
        return status_msg(STATUS_CODE_INTERNAL_ERROR,
                          "SD Card encountered CRC error while writing.\n");
        break;
      case SD_TOKEN_RESPONSE_WRITE_ERROR:
        return status_msg(STATUS_CODE_INTERNAL_ERROR,
                          "SD Card encountered Write error while writing.\n");
        break;
      case SD_TOKEN_RESPONSE_ACCEPT:
        do {
          s = prv_read_byte(spi, &readvalue);
        } while (readvalue != SD_DUMMY_BYTE);
        return STATUS_CODE_OK;
        break;
      default:
        break;
    }
  }
  return status_msg(STATUS_CODE_INTERNAL_ERROR,
                    "Received invalid write Response from the SD Card.\n");
}

static StatusCode prv_send_cmd(SpiPort spi, SdResponse *response_field, uint8_t cmd, uint32_t arg) {
  uint8_t frame[SD_SEND_SIZE];

  // Split the cmd parameter into 8 byte ints
  frame[0] = (cmd | 0x40);
  frame[1] = (uint8_t)(arg >> 24);
  frame[2] = (uint8_t)(arg >> 16);
  frame[3] = (uint8_t)(arg >> 8);
  frame[4] = (uint8_t)(arg);
  frame[5] = (uint8_t)0x95;  // CRC for a CMD0. We can hardcode this since the CRC is ignored from
                             // then on anyways.

  spi_cs_set_state(spi, GPIO_STATE_LOW);

  spi_tx(spi, frame, SD_SEND_SIZE);

  if (cmd == SD_CMD_STOP_TRANSMISSION) {
    // Discard stuff byte in currently transmitting data
    prv_read_byte(spi, NULL);
  }

  uint8_t res[5];
  StatusCode read_result = prv_wait_sd_response(spi, res);
  if (read_result != STATUS_CODE_OK) return read_result;
  response_field->r1 = res[0];
  response_field->ocr_value = *((uint32_t *)(res + 1));
  switch (cmd) {
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
  return STATUS_CODE_OK;
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
    prv_end_transaction(spi);
    if (!(--retry_counter)) {
      return status_msg(STATUS_CODE_TIMEOUT, "Fail to init SD card before timeout\n");
    }
    delay_ms(20);
  } while (!((last_response.r1 & SD_R1_IDLE) && last_status == STATUS_CODE_OK));

  retry_counter = SD_NUM_RETRIES;
  // Send CMD8 (SD_CMD_SEND_IF_COND) to check the power supply status. SDV2 only.
  // and wait until response (R7 Format). If there isn't a response after a set timeout, we'll stop
  // the initialization. Arg: Voltage Supplied is 2.7-3.6V, Check Pattern is 0xAA
  do {
    last_status = prv_send_cmd(spi, &last_response, SD_CMD_SEND_IF_COND, 0x1AA);
    prv_end_transaction(spi);
    if (last_response.r1 & SD_R1_ILLEGAL_CMD) {  // Legacy SD Card
      sd_port[spi].is_sdv2 = false;
      break;
    }
    if (!(--retry_counter)) {
      return status_msg(
          STATUS_CODE_TIMEOUT,
          "SD Card failed to give valid infrastructure condition in alloted number of retries\n");
    }
    if (!(last_response.ocr_value & SD_R7_VAC_BITS)) {
      return status_msg(STATUS_CODE_INTERNAL_ERROR,
                        "SD Card does not support provided voltage range\n");
    }
    if (last_response.ocr_value & (SD_R7_CHECK_PAT == 0xAA)) {
      sd_port[spi].is_sdv2 = true;
      break;
    }
  } while (true);

  // Check voltage again (SD1/SD2)
  last_status = prv_send_cmd(spi, &last_response, SD_CMD_READ_OCR, 0);
  prv_end_transaction(spi);

  if (!(--retry_counter))
    return status_msg(STATUS_CODE_TIMEOUT, "SD Card failed to power within alloted timeframe\n");

  if (last_response.r1 & SD_R1_ILLEGAL_CMD)
    return status_msg(STATUS_CODE_INTERNAL_ERROR,
                      "Attached device does not recognize CMD58, aborting\n");
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
    prv_end_transaction(spi);

    last_status = prv_send_cmd(spi, &last_response, SD_CMD_APP_SEND_OP_COND, 0x40000000);
    prv_end_transaction(spi);
  } while (last_response.r1 & SD_R1_IDLE);

  return STATUS_CODE_OK;
}

StatusCode sd_read_blocks(SpiPort spi, uint8_t *dest, uint32_t read_addr, uint32_t block_count) {
  StatusCode last_status;
  SdResponse last_response;
  uint8_t data_token;

  if (!block_count)
    return status_msg(STATUS_CODE_INVALID_ARGS, "Can't read zero blocks from the SD Card.\n");
  // Note from previous: Read Block Size is locked at 512 bytes

  last_status = prv_send_cmd(spi, &last_response, SD_CMD_READ_MULTIPLE_BLOCK, read_addr);
  if (!(last_response.r1 & SD_R1_NO_ERRORS)) {
    prv_end_transaction(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR,
                      "Failed to read because SD card responded with an error\n");
  }

  // Now look for the data token to signify the start of the data
  do {
    last_status = prv_sd_get_next_data_token(spi, &data_token);
    if (last_status != STATUS_CODE_OK) return last_status;
    if (data_token == SD_TOKEN_START_DATA_MULTI_BLOCK_READ) {
      last_status = prv_read_multi(spi, dest, SD_BLOCK_SIZE);
      if (last_status != STATUS_CODE_OK) return last_status;
      // Burn the two CRC bytes
      prv_read_multi(spi, NULL, 2);
    } else {
      prv_end_transaction(spi);
      if (data_token & SD_TOKEN_ERRORLESS_MASK) {
        return status_msg(STATUS_CODE_INTERNAL_ERROR,
                          "SD Card gave an invalid token for the multi-block operation type\n");
      } else {
        return status_msg(STATUS_CODE_INTERNAL_ERROR,
                          "The SD Card encountered an error while reading.\n");
      }
    }

    dest += SD_BLOCK_SIZE;
  } while (block_count--);
  last_status = prv_send_cmd(spi, &last_response, SD_CMD_STOP_TRANSMISSION, 0);
  prv_end_transaction(spi);
  if (last_status != STATUS_CODE_OK) return last_status;
  if (!(last_response.r1 & SD_R1_NO_ERRORS))
    return status_msg(STATUS_CODE_INTERNAL_ERROR,
                      "An unknown error occurred while stopping transmission. It is recommended "
                      "not to use this SD Card further.\n");
  return STATUS_CODE_OK;
}

// potentially autocalculate number of blocks?
StatusCode sd_write_blocks(SpiPort spi, uint8_t *src, uint32_t write_addr,
                           uint32_t NumberOfBlocks) {
  if (NumberOfBlocks <= 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Number of blocks cannot be 0");
  }

  StatusCode last_status;
  SdResponse response;
  uint8_t datawrite_response = 0;

  // Set Blocklen is redundant since HC SD cards don't support changing block length anyways, and
  // everyone uses 512

  // Data transfer
  last_status = prv_send_cmd(spi, &response, SD_CMD_WRITE_MULTIPLE_BLOCK, write_addr);
  if (last_status != STATUS_CODE_OK) return last_status;
  if (response.r1 != SD_R1_NO_ERRORS) {
    prv_end_transaction(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR,
                      "An error occurred while initializing the card-writing process");
  }

  while (NumberOfBlocks--) {
    // Padding
    last_status = prv_read_byte(spi, NULL);
    if (last_status != STATUS_CODE_OK) return last_status;

    // Send the data token to signify the start of the data
    last_status = prv_write_read_byte(spi, NULL, SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE);
    if (last_status != STATUS_CODE_OK) return last_status;

    // Write the block data to SD
    last_status = spi_tx(spi, src, SD_BLOCK_SIZE);
    if (last_status != STATUS_CODE_OK) return last_status;
    src += SD_BLOCK_SIZE;

    // Put CRC bytes (not really needed by us, but required by SD) (are they? we'll put them there
    // anyways)
    uint16_t crc = crc15_calculate(src, SD_BLOCK_SIZE);
    last_status = spi_tx(spi, (uint8_t *)&crc, 2);
    if (last_status != STATUS_CODE_OK) return last_status;

    last_status = prv_sd_get_next_write_data_response(spi, &datawrite_response);
    if (last_status != STATUS_CODE_OK) return last_status;
    if (!(((datawrite_response & SD_TOKEN_RESPONSE_MASK) == SD_TOKEN_RESPONSE_MASKED_VAL) &&
          ((datawrite_response & SD_TOKEN_RESPONSE_DATA_MASK) == SD_TOKEN_RESPONSE_ACCEPT))) {
      // Quit and return failed status
      last_status = prv_read_byte(spi, NULL);
      if (last_status != STATUS_CODE_OK) return last_status;

      last_status = prv_write_read_byte(spi, NULL, SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE);
      if (last_status != STATUS_CODE_OK) return last_status;

      last_status = prv_read_byte(spi, NULL);
      if (last_status != STATUS_CODE_OK) return last_status;

      uint8_t busy_byte;
      do {
        last_status = prv_read_byte(spi, &busy_byte);
        if (last_status != STATUS_CODE_OK) return last_status;
      } while (!busy_byte);
      prv_end_transaction(spi);
      return status_msg(STATUS_CODE_INTERNAL_ERROR,
                        "An error occurred while writing blocks to the SD Card.");
    }
  }

  prv_read_byte(spi, NULL);  // Spacing
  prv_write_read_byte(spi, NULL, SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE);
  prv_read_byte(spi, NULL);  // Padding byte
  uint8_t busy_byte;
  do {
    prv_read_byte(spi, &busy_byte);
  } while (!busy_byte);
  prv_end_transaction(spi);
  return STATUS_CODE_OK;
}

bool sd_is_initialized(SpiPort spi) {
  StatusCode last_status;
  SdResponse resp;

  last_status = prv_send_cmd(spi, &resp, SD_CMD_SEND_STATUS, 0);
  if (last_status != STATUS_CODE_OK) return false;
  if (resp.r1 != SD_R1_NO_ERRORS) return false;
  return true;
}
