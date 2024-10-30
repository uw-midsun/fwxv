#include "bmi323.h"
static bmi323_storage *s_storage;


static StatusCode prv_set_user_bank(uint8_t user_bank) {
    uint8_t buf[2];
    buf[0] = WRITE & REG_BANK_SEL;
    buf[1] = user_bank;
    StatusCode status = spi_exchange(s_storage->settings->spi_port, buf, sizeof(buf), NULL, 0);
    return status;
}


//read operation requires 1 dummy byte before payload
static StatusCode get_register(bmi323_registers reg, uint16_t *value){
    //register length of 2 bytes, 2nd one is dummy byte
    uint8_t reg8[2];
    reg8[0] = READ | reg;
    reg8[1] = 0x00;
    // prv_set_user_bank(user_bank);
    StatusCode status = spi_exchange(s_storage->settings->spi_port, &reg8, sizeof(uint8_t) * 2, value, sizeof(uint16_t));
    return status;
}

static StatusCode get_multi_register(bmi323_registers reg, uint16_t *reg_val[6], uint8_t len){
    uint8_t reg8[2];
    reg8[0] = READ | reg;
    reg8[1] = 0x00;
    // prv_set_user_bank(user_bank);
    StatusCode status = spi_exchange(s_storage->settings->spi_port, &reg8, sizeof(uint8_t) * 2, reg_val, sizeof(uint16_t) * len);
    return status;
}

static StatusCode set_register(uint16_t reg_addr, uint16_t value) {
    uint16_t tx_buff[2];

    tx_buff[0] = WRITE & reg_addr;
    tx_buff[1] = value;

    StatusCode status = spi_exchange(s_storage->settings->spi_port, tx_buff, sizeof(tx_buff), NULL, 0);

    return status;
}

static StatusCode set_multi_register(uint16_t reg_addr, uint16_t *value, uint16_t len) {
    uint16_t reg_mask = WRITE & reg_addr;

    StatusCode status_addr = spi_exchange(s_storage->settings->spi_port, reg_mask, sizeof(reg_mask), NULL, 0);
    if (status_addr != STATUS_CODE_OK) {
        return status_addr;
    }
        
    StatusCode status_data = spi_exchange(s_storage->settings->spi_port, value, len, NULL, 0);

    return status_data;
}