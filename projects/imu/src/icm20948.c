#include "icm20948.h"

#include "icm20948_defs.h"
#include "log.h"

static float gyro_scale_factor;
static float accel_scale_factor;
static icm20948_storage *s_storage;

static StatusCode prv_set_user_bank(uint8_t ub) {
  uint8_t buf[2];
  buf[0] = WRITE & REG_BANK_SEL;
  buf[1] = ub;
  status_ok_or_return(spi_exchange(s_storage->settings->spi_port, buf, sizeof(buf), NULL, 0));
  return STATUS_CODE_OK;
}

static StatusCode prv_get_reg(uint8_t ub, icm20948_registers reg, uint8_t *value) {
  uint8_t reg8 = READ | reg;
  prv_set_user_bank(ub);
  status_ok_or_return(
      spi_exchange(s_storage->settings->spi_port, &reg8, sizeof(uint8_t), value, sizeof(uint8_t)));
  return STATUS_CODE_OK;
}

static uint8_t *prv_get_multiple_reg(uint8_t ub, icm20948_registers reg, uint8_t len) {
  uint8_t reg8 = READ | reg;
  static uint8_t reg_val[6];
  prv_set_user_bank(ub);
  spi_exchange(s_storage->settings->spi_port, &reg8, sizeof(uint8_t), reg_val,
               (len * sizeof(uint8_t)));
  return reg_val;
}

static StatusCode prv_set_reg(uint8_t ub, icm20948_registers reg, uint8_t value) {
  uint8_t buf[2];
  buf[0] = WRITE & reg;
  buf[1] = value;
  prv_set_user_bank(ub);
  status_ok_or_return(spi_exchange(s_storage->settings->spi_port, buf, sizeof(buf), NULL, 0));
  return STATUS_CODE_OK;
}

static StatusCode prv_set_multiple_reg(uint8_t ub, icm20948_registers reg, uint8_t *value,
                                       uint8_t len) {
  uint8_t reg8 = WRITE & reg;
  prv_set_user_bank(ub);
  status_ok_or_return(spi_exchange(s_storage->settings->spi_port, &reg8, sizeof(reg8), NULL, 0));
  status_ok_or_return(spi_exchange(s_storage->settings->spi_port, value, len, NULL, 0));
  return STATUS_CODE_OK;
}

static StatusCode prv_get_ak09916_reg(icm20948_registers reg, uint8_t *value) {
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_ADDR, READ | MAG_SLAVE_ADDR);
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_REG, reg);
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_CTRL, 0x81);
  delay_ms(1);

  prv_get_reg(0x00, AGB0_REG_EXT_SLV_SENS_DATA_00, value);

  return STATUS_CODE_OK;
}

static StatusCode prv_set_ak09916_reg(icm20948_registers reg, uint8_t value) {
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_ADDR, WRITE & MAG_SLAVE_ADDR);
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_REG, reg);
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_DO, value);
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_CTRL, 0x81);
  return STATUS_CODE_OK;
}

static uint8_t *prv_get_multiple_ak09916_reg(icm20948_registers reg, uint8_t len) {
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_ADDR, READ | MAG_SLAVE_ADDR);
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_REG, reg);
  prv_set_reg(0x30, AGB3_REG_I2C_SLV0_CTRL, 0x80 | len);
  delay_ms(1);

  return prv_get_multiple_reg(0x00, AGB0_REG_EXT_SLV_SENS_DATA_00, len);
}

static StatusCode prv_getAccelData(axises *accel) {
  uint8_t *temp = prv_get_multiple_reg(0x00, AGB0_REG_ACCEL_XOUT_H, 6);
  accel->x = (int16_t)(temp[0] << 8 | temp[1]);
  accel->y = (int16_t)(temp[2] << 8 | temp[3]);
  accel->z = (int16_t)(temp[4] << 8 | temp[5]);
  return STATUS_CODE_OK;
}

static StatusCode prv_getGyroData(axises *gyro) {
  uint8_t *temp = prv_get_multiple_reg(0x00, AGB0_REG_GYRO_XOUT_H, 6);
  gyro->x = (int16_t)(temp[0] << 8 | temp[1]);
  gyro->y = (int16_t)(temp[2] << 8 | temp[3]);
  gyro->z = (int16_t)(temp[4] << 8 | temp[5]);
  return STATUS_CODE_OK;
}

static StatusCode prv_getMagData(axises *mag) {
  uint8_t *temp;
  uint8_t data_rdy, hofl;

  prv_get_ak09916_reg(M_REG_ST1, &data_rdy);
  data_rdy &= 0x01;
  if (!data_rdy) {
    return STATUS_CODE_INCOMPLETE;
  }

  temp = prv_get_multiple_ak09916_reg(M_REG_HXL, 6);

  prv_get_ak09916_reg(M_REG_ST2, &hofl);
  hofl &= 0x08;

  if (hofl) {
    return STATUS_CODE_INCOMPLETE;
  }

  mag->x = (int16_t)(temp[1] << 8 | temp[0]);
  mag->y = (int16_t)(temp[3] << 8 | temp[2]);
  mag->z = (int16_t)(temp[5] << 8 | temp[4]);

  return STATUS_CODE_OK;
}

static bool prv_who_am_i() {
  uint8_t reg;
  prv_get_reg(0x00, AGB0_REG_WHO_AM_I, &reg);

  if (reg == ICM20948_ID) {
    return true;
  } else {
    return false;
  }
}

static bool prv_ak09916_who_am_i() {
  uint8_t ak09916_id;
  prv_get_ak09916_reg(M_REG_WIA2, &ak09916_id);
  if (ak09916_id == AK09916_ID) {
    return true;
  } else {
    return false;
  }
}

static StatusCode icm20948_reset() {
  uint8_t new_val;

  prv_set_reg(0x00, AGB0_REG_PWR_MGMT_1, 0x80 | 0x45);
  delay_ms(100);

  new_val = prv_get_reg(0x00, AGB0_REG_PWR_MGMT_1, &new_val);
  new_val &= 0xBF;
  prv_set_reg(0x00, AGB0_REG_PWR_MGMT_1, new_val);
  delay_ms(100);

  return STATUS_CODE_OK;
}

static StatusCode ak09916_reset(ak09916_mode mode) {
  uint8_t reg;
  prv_set_ak09916_reg(M_REG_CNTL3, 0x01);
  delay_ms(100);
  prv_set_ak09916_reg(M_REG_CNTL2, mode);
  delay_ms(100);

  return STATUS_CODE_OK;
}

static StatusCode icm20948_i2c_master_reset() {
  uint8_t reg;
  // Enables all I2C and enables SPI only mode
  prv_get_reg(0x00, AGB0_REG_USER_CTRL, &reg);
  prv_set_reg(0x00, AGB0_REG_USER_CTRL, (reg | 0x02));

  prv_get_reg(0x00, AGB0_REG_USER_CTRL, &reg);
  prv_set_reg(0x00, AGB0_REG_USER_CTRL, reg | 0x20);
  delay_ms(100);

  return STATUS_CODE_OK;
}

static StatusCode icm20948_i2c_master_clk_frq(uint8_t config) {
  uint8_t new_val;
  prv_get_reg(0x30, AGB3_REG_I2C_MST_CTRL, &new_val);
  new_val |= config;

  prv_set_reg(0x30, AGB3_REG_I2C_MST_CTRL, new_val);

  return STATUS_CODE_OK;
}

static StatusCode icm20948_accel_low_pass(uint8_t low_pass_config) {
  uint8_t new_val;
  prv_get_reg(0x20, AGB2_REG_ACCEL_CONFIG_1, &new_val);
  new_val |= 0x01;
  new_val |= low_pass_config << 3;

  prv_set_reg(0x20, AGB2_REG_ACCEL_CONFIG_1, new_val);
  return STATUS_CODE_OK;
}

static StatusCode icm20948_gyro_low_pass(uint8_t low_pass_config) {
  uint8_t new_val;
  prv_get_reg(0x20, AGB2_REG_GYRO_CONFIG_1, &new_val);
  new_val |= 0x01;
  new_val |= low_pass_config << 3;

  prv_set_reg(0x20, AGB2_REG_GYRO_CONFIG_1, new_val);
  return STATUS_CODE_OK;
}

static StatusCode icm20948_accel_calibration() {
  axises calib;
  int32_t accel_bias[3] = { 0 };
  int32_t accel_bias_regs[3];
  uint8_t mask_bit[3];
  uint8_t accel_offset[6] = { 0 };

  // Collect 200 samples of accelerometer data
  for (uint8_t i = 0; i < 200; i++) {
    prv_getAccelData(&calib);
    accel_bias[0] += calib.x;
    accel_bias[1] += calib.y;
    accel_bias[2] += calib.z;
  }

  accel_bias[0] /= 200;
  accel_bias[1] /= 200;
  accel_bias[2] /= 200;

  uint8_t *temp;

  temp = prv_get_multiple_reg(0x10, AGB1_REG_XA_OFFS_H, 2);
  accel_bias_regs[0] = (int32_t)(temp[0] << 8 | temp[1]);
  mask_bit[0] = temp[1] & 0x01;

  temp = prv_get_multiple_reg(0x10, AGB1_REG_YA_OFFS_H, 2);
  accel_bias_regs[1] = (int32_t)(temp[0] << 8 | temp[1]);
  mask_bit[1] = temp[1] & 0x01;

  temp = prv_get_multiple_reg(0x10, AGB1_REG_ZA_OFFS_H, 2);
  accel_bias_regs[2] = (int32_t)(temp[0] << 8 | temp[1]);
  mask_bit[2] = temp[1] & 0x01;

  accel_bias_regs[0] -= (accel_bias[0] / 8);
  accel_bias_regs[1] -= (accel_bias[1] / 8);
  accel_bias_regs[2] -= (accel_bias[2] / 8);

  accel_offset[0] = (accel_bias_regs[0] >> 8) & 0xFF;
  accel_offset[1] = ((accel_bias_regs[0]) & 0xFE) | mask_bit[0];
  accel_offset[2] = (accel_bias_regs[1] >> 8) & 0xFF;
  accel_offset[3] = ((accel_bias_regs[1]) & 0xFE) | mask_bit[1];
  accel_offset[4] = (accel_bias_regs[2] >> 8) & 0xFF;
  accel_offset[5] = ((accel_bias_regs[2]) & 0xFE) | mask_bit[2];

  prv_set_multiple_reg(0x10, AGB1_REG_XA_OFFS_H, accel_offset, 6);

  return STATUS_CODE_OK;
}

static StatusCode icm20948_gyro_calibration() {
  axises calib;
  int32_t gyro_bias[3] = { 0 };
  uint8_t gyro_offset[6] = { 0 };

  for (uint8_t i = 0; i < 200; i++) {
    prv_getGyroData(&calib);
    gyro_bias[0] += calib.x;
    gyro_bias[1] += calib.y;
    gyro_bias[2] += calib.z;
  }
  gyro_bias[0] /= 200;
  gyro_bias[1] /= 200;
  gyro_bias[2] /= 200;

  gyro_offset[0] = (-gyro_bias[0] / 4 >> 8) & 0xFF;
  gyro_offset[1] = (-gyro_bias[0] / 4) & 0xFF;
  gyro_offset[2] = (-gyro_bias[1] / 4 >> 8) & 0xFF;
  gyro_offset[3] = (-gyro_bias[1] / 4) & 0xFF;
  gyro_offset[4] = (-gyro_bias[2] / 4 >> 8) & 0xFF;
  gyro_offset[5] = (-gyro_bias[2] / 4) & 0xFF;

  prv_set_multiple_reg(0x20, AGB2_REG_XG_OFFS_USRH, gyro_offset, 6);

  return STATUS_CODE_OK;
}

static StatusCode prv_set_accel_range(icm20948_accel_range range) {
  uint8_t new_val;
  prv_get_reg(0x20, AGB2_REG_ACCEL_CONFIG_1, &new_val);

  switch (range) {
    case ICM20948_ACCEL_RANGE_2_G:
      new_val |= 0x00;
      accel_scale_factor = 16384;
      break;
    case ICM20948_ACCEL_RANGE_4_G:
      new_val |= 0x02;
      accel_scale_factor = 8192;
      break;
    case ICM20948_ACCEL_RANGE_8_G:
      new_val |= 0x04;
      accel_scale_factor = 4096;
      break;
    case ICM20948_ACCEL_RANGE_16_G:
      new_val |= 0x06;
      accel_scale_factor = 2048;
      break;
  }

  prv_set_reg(0x20, AGB2_REG_ACCEL_CONFIG_1, new_val);
  return STATUS_CODE_OK;
}

static StatusCode prv_set_gyro_range(icm20948_gyro_range range) {
  uint8_t new_val;
  prv_get_reg(0x20, AGB2_REG_GYRO_CONFIG_1, &new_val);

  switch (range) {
    case ICM20948_GYRO_RANGE_250_DPS:
      new_val |= 0x00;
      gyro_scale_factor = 131.0;
      break;
    case ICM20948_GYRO_RANGE_500_DPS:
      new_val |= 0x02;
      gyro_scale_factor = 65.5;
      break;
    case ICM20948_GYRO_RANGE_1000_DPS:
      new_val |= 0x04;
      gyro_scale_factor = 32.8;
      break;
    case ICM20948_GYRO_RANGE_2000_DPS:
      new_val |= 0x06;
      gyro_scale_factor = 16.4;
      break;
  }

  prv_set_reg(0x20, AGB2_REG_GYRO_CONFIG_1, new_val);
  return STATUS_CODE_OK;
}

StatusCode icm20948_getAccelData() {
  prv_getAccelData(&s_storage->accel);
  s_storage->accel.x *= (ACCELERATION_DUE_TO_GRAVITY_MM) / accel_scale_factor;
  s_storage->accel.y *= (ACCELERATION_DUE_TO_GRAVITY_MM) / accel_scale_factor;
  s_storage->accel.z *= (ACCELERATION_DUE_TO_GRAVITY_MM) / accel_scale_factor;
  return STATUS_CODE_OK;
}

StatusCode icm20948_getGyroData() {
  prv_getGyroData(&s_storage->gyro);
  s_storage->gyro.x /= gyro_scale_factor;
  s_storage->gyro.y /= gyro_scale_factor;
  s_storage->gyro.z /= gyro_scale_factor;
  return STATUS_CODE_OK;
}

StatusCode icm20948_getMagData() {
  status_ok_or_return(prv_getMagData(&s_storage->mag));

  s_storage->mag.x = (float)(s_storage->mag.x * 0.15);
  s_storage->mag.y = (float)(s_storage->mag.y * 0.15);
  s_storage->mag.z = (float)(s_storage->mag.z * 0.15);
  return STATUS_CODE_OK;
}

StatusCode icm20948_init(icm20948_storage *storage, icm20948_settings *settings) {
  s_storage = storage;
  SpiSettings spi_settings = {
    .baudrate = settings->spi_baudrate,
    .mode = SPI_MODE_3,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->sclk,
    .cs = settings->cs,
  };
  spi_init(settings->spi_port, &spi_settings);

  uint8_t reg;
  while (!prv_who_am_i()) {
  }
  // Clears all internal registers + Disables temperature sensor + resets to default settings
  icm20948_reset();

  // Enables ODR time enabling to ensure all measurements are synchronized and consistent
  prv_set_reg(0x20, AGB2_REG_ODR_ALIGN_EN, 0x01);

  // Disables all slave I2C and enables SPI only mode
  prv_get_reg(0x00, AGB0_REG_USER_CTRL, &reg);
  prv_set_reg(0x00, AGB0_REG_USER_CTRL, reg | 0x10);

  icm20948_gyro_low_pass(1);
  icm20948_accel_low_pass(1);

  icm20948_gyro_calibration();
  icm20948_accel_calibration();

  prv_set_gyro_range(s_storage->settings->gyro_range);
  prv_set_accel_range(s_storage->settings->accel_range);

  icm20948_i2c_master_reset();
  icm20948_i2c_master_clk_frq(7);

  while (!prv_ak09916_who_am_i()) {
  }

  ak09916_reset(s_storage->settings->mag_mode);

  return STATUS_CODE_OK;
}
