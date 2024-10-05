#include "bmi323.h"
static bmi323_storage *s_storage;


static StatusCode set_register(SpiPort spi, uint8_t reg_addr, uint8_t value) {
    uint8_t tx_buff[2];

    tx_buff[0] = WRITE & reg_addr;
    tx_buff[1] = value;

    StatusCode status = spi_exchange(spi, tx_buff, sizeof(tx_buff), NULL, 0);

    return status;
}

static StatusCode set_multi_register(SpiPort spi, uint8_t reg_addr, uint8_t *value, uint8_t len) {
    uint8_t reg_mask = WRITE & reg_addr;

    StatusCode status_addr = spi_exchange(spi, reg_mask, sizeof(reg_mask), NULL, 0);
    if (status_addr != STATUS_CODE_OK) {
        return status_addr;
    }

    StatusCode status_data = spi_exchange(spi, value, len, NULL, 0);

    return status_data;
}

static StatusCode prv_set_user_bank(uint8_t ub) {
    uint8_t buf[2];
    buf[0] = WRITE & REG_BANK_SEL;
    buf[1] = ub;
    status_ok_or_return(spi_exchange(s_storage->settings->spi_port, buf, sizeof(buf), NULL, 0));
    return STATUS_CODE_OK;
}

static StatusCode prv_get_reg(uint8_t ub, bmi323_registers reg, uint8_t *value){
    uint8_t reg8 = READ | reg;
    prv_set_user_bank(ub);
    status_ok_or_return(spi_exchange(s_storage->settings->spi_port, &reg8, sizeof(uint8_t), value, sizeof(uint8_t)));
    return STATUS_CODE_OK;
}