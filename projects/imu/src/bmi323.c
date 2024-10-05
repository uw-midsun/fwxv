#include "bmi323.h"
#include "log.h"

static bmi323_storage *s_storage;

static StatusCode prv_set_user_bank(uint8_t ub){
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