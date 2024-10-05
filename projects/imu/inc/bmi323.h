#pragma once

#include "log.h"
#include "spi.h"


#define READ 0x80
#define WRITE 0x7F
#define DUMMY_BYTE = 0x00

typedef enum{
  REG_BANK_SEL = 0x7F,
}bmi323_registers;

typedef struct {
  SpiPort spi_port;
}bmi323_settings;

typedef struct {
  bmi323_settings *settings;
} bmi323_storage;


static StatusCode set_register(uint8_t user_bank, uint8_t reg_addr, uint8_t value);
static StatusCode set_multi_register(uint8_t user_bank, uint8_t reg_addr, uint8_t *value, uint8_t len);
static StatusCode get_register(bmi323_registers reg, uint8_t *value);
static StatusCode get_multi_register(bmi323_registers reg, uint16_t *reg_val[6], uint8_t len);