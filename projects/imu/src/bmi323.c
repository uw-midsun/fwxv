#include "bmi323.h"

static bmi323_storage *s_storage;


//read operation requires 1 dummy byte before payload
static StatusCode get_register(bmi323_registers reg, uint16_t *value){
    //register length of 2 bytes, 2nd one is dummy byte
    uint8_t reg8[2];
    reg8[0] = READ | reg;
    reg8[1] = DUMMY_BYTE;
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

static StatusCode get_gyro_offset_gain(struct gyro_gain_offset *gyro_go){
    StatusCode result;
    uint8_t data[12] = {0};

    uint16_t gyro_off_x, gyro_off_y, gyro_off_z;
    uint8_t gyro_gain_x, gyro_gain_y, gyro_gain_z;

    if(gyro_go != NULL){
        result = get_multi_register(GYR_DP_OFF_X, data, 12);

        if(result == STATUS_CODE_OK){
            gyro_off_x = (uint16_t)(data[1] << 8 | data[0]);
            gyro_gain_x = (uint8_t)(data[2]);
            gyro_off_y = (uint16_t)(data[5] << 8 | data[4]);
            gyro_gain_y = (uint8_t)(data[6]);
            gyro_off_z = (uint16_t)(data[9] << 8 | data[8]);
            gyro_gain_z = (uint8_t)(data[10]);

            gyro_go -> gyro_offset_x = gyro_off_x & BMI3_GYR_DP_OFF_X_MASK;
            gyro_go -> gyro_gain_x = gyro_gain_x & BMI3_GYR_DP_DGAIN_X_MASK;
            gyro_go -> gyro_offset_y = gyro_off_y & BMI3_GYR_DP_OFF_Y_MASK;
            gyro_go -> gyro_gain_y = gyro_gain_y & BMI3_GYR_DP_DGAIN_Y_MASK;
            gyro_go -> gyro_offset_z = gyro_off_z & BMI3_GYR_DP_OFF_Z_MASK;
            gyro_go -> gyro_gain_z = gyro_gain_z & BMI3_GYR_DP_DGAIN_Z_MASK;
            
        }

    }else{
        result = BMI3_E_NULL_PTR;
    }

    return result;
}

static StatusCode set_gyro_offset_gain(struct gyro_gain_offset *gyro_go){
    StatusCode result;
    uint8_t data[12] = {0};

    uint16_t gyro_off_x, gyro_off_y, gyro_off_z;
    uint8_t gyro_gain_x, gyro_gain_y, gyro_gain_z;

    if(gyro_go != NULL){
        gyro_off_x = gyro_go -> gyro_offset_x & BMI3_GYR_DP_OFF_X_MASK;
        gyro_gain_x = gyro_go -> gyro_gain_x & BMI3_GYR_DP_DGAIN_X_MASK;
        gyro_off_y = gyro_go -> gyro_offset_y & BMI3_GYR_DP_OFF_Y_MASK;
        gyro_gain_y = gyro_go -> gyro_gain_y & BMI3_GYR_DP_DGAIN_Y_MASK;
        gyro_off_z = gyro_go -> gyro_offset_z & BMI3_GYR_DP_OFF_Z_MASK;
        gyro_gain_z = gyro_go -> gyro_gain_z & BMI3_GYR_DP_DGAIN_Z_MASK;

        data[0] = (uint8_t)(gyro_off_x & BMI_SET_LOW_BYTE);
        data[1] = (uint8_t)(gyro_off_x & BMI_SET_HIGH_BYTE) >> 8;
        data[2] = (uint8_t)(gyro_gain_x);
        data[4] = (uint8_t)(gyro_off_y & BMI_SET_LOW_BYTE);
        data[5] = (uint8_t)(gyro_off_y & BMI_SET_HIGH_BYTE) >> 8;
        data[6] = (uint8_t)(gyro_gain_y);
        data[8] = (uint8_t)(gyro_off_z & BMI_SET_LOW_BYTE);
        data[9] = (uint8_t)(gyro_off_z & BMI_SET_HIGH_BYTE) >> 8;
        data[10] = (uint8_t)(gyro_gain_z);

        result = set_multi_register(GYR_DP_OFF_X, data, 12);
    }else{
        result = BMI3_E_NULL_PTR;
    }

    return result;
}

static StatusCode get_accel_offset_gain(struct accel_gain_offset *accel_go){
    StatusCode result;
    uint8_t data[12] = {0};

    uint16_t accel_off_x, accel_off_y, accel_off_z;
    uint8_t accel_gain_x, accel_gain_y, accel_gain_z;

    if(accel_go != NULL){
        result = get_multi_register(ACC_DP_OFF_X, data, 12);

        if(result == STATUS_CODE_OK){
            accel_off_x = (uint16_t)(data[1] << 8 | data[0]);
            accel_gain_x = (uint8_t)data[2];
            accel_off_y = (uint16_t)(data[5] << 8 | data[4]);
            accel_gain_y = (uint8_t)data[6];
            accel_off_z = (uint16_t)(data[9] << 8 | data[8]);
            accel_gain_z = (uint8_t)data[10];

            accel_go -> accel_offset_x = (accel_off_x & BMI3_ACC_DP_DOFFSET_X_MASK);
            accel_go -> accel_gain_x = (accel_gain_x & BMI3_ACC_DP_DGAIN_X_MASK);
            accel_go -> accel_offset_y = (accel_off_y & BMI3_ACC_DP_DOFFSET_Y_MASK);
            accel_go -> accel_gain_y = (accel_gain_y & BMI3_ACC_DP_DGAIN_Y_MASK);
            accel_go -> accel_offset_z = (accel_off_z & BMI3_ACC_DP_DOFFSET_Z_MASK);
            accel_go -> accel_gain_z = (accel_gain_z & BMI3_ACC_DP_DGAIN_Z_MASK);
        }
    }else{
        result = BMI3_E_NULL_PTR;
    }

    return result;
}

static StatusCode set_accel_offset_gain(struct accel_gain_offset *accel_go){
    StatusCode result;

    uint8_t data[12] = {0};
    
    uint16_t accel_off_x, accel_off_y, accel_off_z;
    uint8_t accel_gain_x, accel_gain_y, accel_gain_z;
    if(accel_go != NULL){
        accel_off_x = accel_go -> accel_offset_x & BMI3_ACC_DP_DOFFSET_X_MASK;
        accel_gain_x = accel_go -> accel_gain_x & BMI3_ACC_DP_DGAIN_X_MASK;
        accel_off_y = accel_go -> accel_offset_y & BMI3_ACC_DP_DOFFSET_Y_MASK;
        accel_gain_y = accel_go -> accel_gain_y & BMI3_ACC_DP_DGAIN_Y_MASK;
        accel_off_z = accel_go -> accel_offset_z & BMI3_ACC_DP_DOFFSET_Z_MASK;
        accel_gain_z = accel_go -> accel_gain_z & BMI3_ACC_DP_DGAIN_Z_MASK;

        data[0] = (uint8_t)(accel_off_x & BMI_SET_LOW_BYTE);
        data[1] = (uint8_t)(accel_off_x & BMI_SET_HIGH_BYTE) >> 8;
        data[2] = (uint8_t)(accel_gain_x);
        data[4] = (uint8_t)(accel_off_y & BMI_SET_LOW_BYTE);
        data[5] = (uint8_t)(accel_off_y & BMI_SET_HIGH_BYTE) >> 8;
        data[6] = (uint8_t)(accel_gain_y);
        data[8] = (uint8_t)(accel_off_z & BMI_SET_LOW_BYTE);
        data[9] = (uint8_t)(accel_off_z & BMI_SET_HIGH_BYTE) >> 8;
        data[10] = (uint8_t)(accel_gain_z);
        result = set_multi_register(ACC_DP_OFF_X, data, 12);
    }else{
        result = BMI3_E_NULL_PTR;
    }

    return result;
}

/*

EXT.GYR_SC_SELECT.sens_en
EXT.GYR_SC_SELECT.offs_en
if both are being calibrated, sens is first

write 0b0 to change default configuration

by default, results of the self-calibration are written to data path registers:
    GYR_DP_DGAIN_X
    GYR_DP_DGAIN_Y
    GYR_DP_DGAIN_Z
    GYR_DP_OFF_X
    GYR_DP_OFF_Y
    GYR_DP_OFF_Z

    by setting EXT.GYR_SC_SELECT.apply_corr to 0b0, the update of the data path registers can be suppressed
    sensitivity of motion detection can be configured through EXT.GYR_MOT_DET.slope

    prerequisites for self calibration:
        if FEATURE_IO1.state is 0b00, self calibration can be initiated
        accelerometer must be in high performace mode
        sample rate of accelerometer is preffered to be in the range of 25Hz to 200Hz
        alternative sensor configurations must be disabled by:
            ALT_ACC_CONF.alt_acc_mode
            ALT_GYR_CONF.alt_gyr_mode
        to 0b0

    to execute self calibration:
        write 0x0101 to the register CMD
        state of self calibration can be determined by checking
            FEATURE_IO1.sc_st_complete
                0b0 -> ongoing
                0b1 -> completed
                    or
            FEATURE_IO1.state
                0b01 -> ongoing
            FEATURE_IO1.error_status
                0x5 -> completed
        Success of the sef calibration
            FEATURE_IO1.gyro_sc_result
                0b1 -> success
                0b0 -> failure
*/

static StatusCode gyro_crt_calibration(){
    StatusCode enable_status = enable_feature_engine();

    if(enable_status == STATUS_CODE_OK){
        
        //poll int status register value
        uint16_t int_status_int1;
        get_register(INT_STATUS_INT1, int_status_int1);
        uint16_t int_status_int2;
        get_register(INT_STATUS_INT2, int_status_int2);

        int_status_int1 &= (1<<10);
        int_status_int2 &= (1<<10);

        while(int_status_int1 != 1 && int_status_int2 != 1){
            get_register(INT_STATUS_INT1, int_status_int1);
            get_register(INT_STATUS_INT2, int_status_int2);

            int_status_int1 &= (1<<10);
            int_status_int2 &= (1<<10);            
        }

        //read FEATURE_IO1 and check error status
        uint16_t feat_state;
        get_register(FEATURE_IO1, feat_state);

        //error status
        feat_state &= 1111;

        while(feat_state != 0x5){
            get_register(FEATURE_IO1, feat_state);
            feat_state &= 1111;
        }

        //run the calibration
        set_register(CMD, 0x0101);
        

        //wait until self calibration is complete
        get_register(FEATURE_IO1, feat_state);
        feat_state &= (1<<6);

        while(feat_state != 0b1){
            get_register(FEATURE_IO1, feat_state);
            feat_state &= (1<<6);
        }

        //poll int status register value
        get_register(INT_STATUS_INT1, int_status_int1);
        get_register(INT_STATUS_INT2, int_status_int2);
        int_status_int1 &= (1<<10);
        int_status_int2 &= (1<<10);

        while(int_status_int1 != 1 && int_status_int2 != 1){
            get_register(INT_STATUS_INT1, int_status_int1);
            get_register(INT_STATUS_INT2, int_status_int2);
            int_status_int1 &= (1<<10);
            int_status_int2 &= (1<<10);
        }

        //read FEATURE_IO1 and check sc_st_complete and gyro_sc_result
        get_register(FEATURE_IO1, feat_state);

        feat_state &= (1<<4);

        while(feat_state != 1){
            get_register(FEATURE_IO1, feat_state);
            feat_state &= (1<<4);
        }

        get_register(FEATURE_IO1, feat_state);
        feat_state &= (1<<5);

        while(feat_state != 1){
            get_register(FEATURE_IO1, feat_state);
            feat_state &= (1<<5);
        }

        //also check error status if necessary

    }

}

static StatusCode enable_feature_engine(){
    set_register(FEATURE_IO2, 0x012C);
    set_register(FEATURE_IO_STATUS, 0x0001);

    uint16_t feature_ctrl;

    get_register(FEATURE_CTRL, feature_ctrl);

    //set FEATURE_CTRL.engine_en to 0b1
    feature_ctrl &= 1;

    set_register(FEATURE_CTRL, feature_ctrl);

    uint16_t feat_state;

    get_register(FEATURE_IO1, feat_state);

    uint16_t state = feat_state & (0b1111);

    while(state != 0b001){
        //add a timeout here
    }

    return STATUS_CODE_OK;//should only return if we didnt timeout
}
