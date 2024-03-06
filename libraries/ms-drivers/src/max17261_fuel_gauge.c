#include "max17261_fuel_gauge.h"

// See Table 3 on pg.18 of the datasheet
#define PCT_LSB (1.0f / 256)                                  // LSBit is 1/256%
#define CAP_LSB (5.0f / storage->settings->r_sense_mohms)     // LSBit is 5 micro Volt hrs / Rsense
#define TIM_LSB (5625U)                                       // LSBit is 5625ms
#define CUR_LSB (1.5625f / storage->settings->r_sense_mohms)  // LSBit is 1.5625uA / Rsense
#define VOLT_LSB (1.25f / 16)                                 // LSBit is 1.25mV / 16
#define TEMP_LSB (1.0f / 256)                                 // LSBit is 1 / 256 C

static StatusCode max17261_get_reg(Max17261Storage *storage, Max17261Registers reg,
                                   uint16_t *value) {
  // unsure of underlying type of enum, cast to uint8_t to be sure
  uint8_t reg8 = reg;
  status_ok_or_return(
      i2c_write(storage->settings->i2c_port, storage->settings->i2c_address, &reg8, 1));
  // TODO: max17261 sends LSByte then MSByte, need to check if bytes are correctly written to
  status_ok_or_return(i2c_read(storage->settings->i2c_port, storage->settings->i2c_address,
                               (uint8_t *)value, sizeof(uint16_t)));
  return STATUS_CODE_OK;
}

static StatusCode max17261_set_reg(Max17261Storage *storage, Max17261Registers reg,
                                   uint16_t value) {
  uint8_t buf[3];
  buf[0] = reg;
  // send LSByte then MSByte as per datasheet
  buf[1] = value & 0x00FF;
  buf[2] = value >> 8;
  status_ok_or_return(
      i2c_write(storage->settings->i2c_port, storage->settings->i2c_address, buf, sizeof(buf)));
  return STATUS_CODE_OK;
}

StatusCode max17261_state_of_charge(Max17261Storage *storage, uint16_t *soc_pct) {
  uint16_t soc_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_SOC, &soc_reg_val));
  *soc_pct = soc_reg_val * PCT_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_remaining_capacity(Max17261Storage *storage, uint32_t *rem_cap_uAhr) {
  uint16_t rem_cap_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_CAP, &rem_cap_reg_val));
  *rem_cap_uAhr = rem_cap_reg_val * CAP_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_full_capacity(Max17261Storage *storage, uint16_t *full_cap_uAhr) {
  uint16_t full_cap_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_FULL_CAP_REP, &full_cap_reg_val));
  *full_cap_uAhr = full_cap_reg_val * CAP_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_time_to_empty(Max17261Storage *storage, uint16_t *tte_ms) {
  uint16_t tte_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_TIME_TO_EMPTY, &tte_reg_val));
  *tte_ms = tte_reg_val * TIM_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_time_to_full(Max17261Storage *storage, uint16_t *ttf_ms) {
  uint16_t ttf_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_TIME_TO_FULL, &ttf_reg_val));
  *ttf_ms = ttf_reg_val * TIM_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_current(Max17261Storage *storage, uint16_t *current_ua) {
  uint16_t current_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_CURRENT, &current_reg_val));
  *current_ua = current_reg_val * CUR_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_voltage(Max17261Storage *storage, uint16_t *vcell_mv) {
  uint16_t vcell_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_VCELL, &vcell_reg_val));
  *vcell_mv = vcell_reg_val * VOLT_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_temp(Max17261Storage *storage, uint16_t *temp_c) {
  uint16_t temp_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_TEMP, &temp_reg_val));
  *temp_c = temp_reg_val * temp_reg_val;
  return STATUS_CODE_OK;
}

StatusCode max17261_init(Max17261Storage *storage, Max17261Settings *settings) {
  if (settings->i2c_port >= NUM_I2C_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->settings = settings;

  // Configuration for alerts
  // Enable current, voltage, and temperature alerts (pg.17 of datasheet, Config (1Dh) Format)
  uint16_t config = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_CONFIG, &config));
  config |= (1 << 2);
  status_ok_or_return(max17261_set_reg(storage, MAX17261_CONFIG, config));
  // Upper byte is IMAX and lower byte is IMIN
  uint16_t current_th = (settings->i_thresh_max << 8) & (settings->i_thresh_min & 0x00FF);
  status_ok_or_return(max17261_set_reg(storage, MAX17261_I_ALRT_TH, current_th));
  // Upper byte is TMAX and lower byte is TMIN (leave TMIN as 0 C)
  status_ok_or_return(
      max17261_set_reg(storage, MAX17261_TEMP_ALRT_THRSH, (settings->temp_thresh_max << 8)));

  // Make sure voltage alerts are disabled (handled by AFEs) (see datasheet pg.25 for disabled
  // VAlrtTh value)
  status_ok_or_return(max17261_set_reg(storage, MAX17261_VOLT_ALRT_THRSH, (0xFF00)));
  // Make sure SOC alerts are disabled (see datasheet pg.26 for disabled SAlrtTh value)
  status_ok_or_return(max17261_set_reg(storage, MAX17261_SOC_ALRT_THRSH, (0xFF00)));

  status_ok_or_return(max17261_set_reg(storage, MAX17261_DESIGN_CAP, settings->design_capacity));
  status_ok_or_return(
      max17261_set_reg(storage, MAX17261_I_CHG_TERM, settings->charge_term_current));
  uint16_t old_vempty = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_V_EMPTY, &old_vempty));
  // the 7 LSBits of the vempty reg is the recovery voltage, the last 9 are the empty voltage
  uint16_t new_vempty = (settings->empty_voltage << 7) + (old_vempty & 0x7F);
  status_ok_or_return(max17261_set_reg(storage, MAX17261_V_EMPTY, new_vempty));

  return STATUS_CODE_OK;
}
