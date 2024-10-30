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
    uint16_t data[6];

    StatusCode status = get_multi_register(GYRO_REG_ADDR, &data, 6);

    if (status == STATUS_CODE_OK) {
        //(int16_t)((msb << 8) | lsb);
        gyro->x = (int16_t)((data[0] << 8) | data[1]);
        gyro->y = (int16_t)((data[2] << 8) | data[3]);
        gyro->z = (int16_t)((data[4] << 8) | data[5]);

        return STATUS_CODE_OK;
    }

    return status;
}

static StatusCode get_accel_data(axes *accel){
    uint16_t data[6];
    StatusCode status = get_multi_register(ACCEL_REG_ADDR, &data, 6);

    if(status = STATUS_CODE_OK){
        accel->x = (int16_t)((data[0] << 8) | data[1]);
        accel->y = (int16_t)((data[2] << 8) | data[3]);
        accel->z = (int16_t)((data[4] << 8) | data[5]);
    }


    return status;
}

static StatusCode get_accel_offset_gain(struct accel_gain_offset *accel_go){
    StatusCode result;
    uint8_t data[12] = {0};

    uint16_t accel_off_x, accel_off_y, accel_off_z;
    uint8_t accel_gain_x, accel_gain_y, accel_gain_z;

    if(accel_go != NULL){
        result = get_multi_register(ACC_DP_OFF_X, data, 12);

        if(result == STATUS_CODE_OK){
            accel_off_x = (uint16_t)(data[0] << 8 | data[1]);
            accel_gain_x = (uint16_t)data[2];
            accel_off_y = (uint16_t)(data[4] << 8 | data[5]);
            accel_gain_y = (uint16_t)data[6];
            accel_off_z = (uint16_t)(data[8] << 8 | data[9]);
            accel_gain_z = (uint16_t)data[10];

            accel_go -> accel_offset_x = (accel_off_x & BMI3_ACC_DP_DOFFSET_X_MASK);
            accel_go -> accel_gain_x = (accel_gain_x & BMI3_ACC_DP_DGAIN_X_MASK);
            accel_go -> accel_offset_y = (accel_off_y & BMI3_ACC_DP_DOFFSET_Y_MASK);
            accel_go -> accel_gain_y = (accel_gain_y & BMI3_ACC_DP_DGAIN_Y_MASK);
            accel_go -> accel_offset_z = (accel_off_z & BMI3_ACC_DP_DOFFSET_Z_MASK);
            accel_go -> accel_gain_z = (accel_gain_z & BMI3_ACC_DP_DGAIN_Z_MASK);
        }
    }

    return result;
}

static StatusCode set_accel_offset_gain(struct accel_gain_offset *accel_go){
    StatusCode result;

    uint8_t data[12] = {0};
    
    uint16_t accel_off_x, accel_off_y, accel_off_z;
    uint8_t accel_gain_x, accel_gain_y, accel_gain_z;

    accel_off_x = accel_go -> accel_offset_x & BMI3_ACC_DP_DOFFSET_X_MASK;
    accel_gain_x = accel_go -> accel_gain_x & BMI3_ACC_DP_DGAIN_X_MASK;
    accel_off_y = accel_go -> accel_offset_y & BMI3_ACC_DP_DOFFSET_Y_MASK;
    accel_gain_y = accel_go -> accel_gain_y & BMI3_ACC_DP_DGAIN_Y_MASK;
    accel_off_z = accel_go -> accel_offset_z & BMI3_ACC_DP_DOFFSET_Z_MASK;
    accel_gain_z = accel_go -> accel_gain_z & BMI3_ACC_DP_DGAIN_Z_MASK;

    data[0] = (uint8_t)(accel_off_x & 0x00FF);
    data[1] = (uint8_t)(accel_off_x & 0xFF00) >> 8;
    data[2] = (uint8_t)(accel_gain_x);
    data[4] = (uint8_t)(accel_off_y & 0x00FF);
    data[5] = (uint8_t)(accel_off_y & 0xFF00) >> 8;
    data[6] = (uint8_t)(accel_gain_y);
    data[8] = (uint8_t)(accel_off_z & 0x00FF);
    data[9] = (uint8_t)(accel_off_z & 0xFF00) >> 8;
    data[10] = (uint8_t)(accel_gain_z);

    result = set_multi_register(ACC_DP_OFF_X, data, 12);

    return result;
}