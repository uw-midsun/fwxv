#include "bmi323.h"
static bmi323_storage *s_storage;


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

static StatusCode get_gyroscope_data(axes *gyro) {
    uint16_t temp[6];

    StatusCode status = get_multi_register(GYRO_REG_ADDR, &temp, 6);

    if (status == STATUS_CODE_OK) {
        //(int16_t)((msb << 8) | lsb);
        gyro->x = (int16_t)((temp[0] << 8) | temp[1]);
        gyro->y = (int16_t)((temp[2] << 8) | temp[3]);
        gyro->z = (int16_t)((temp[4] << 8) | temp[5]);

        return STATUS_CODE_OK;
    }

    return status;
}