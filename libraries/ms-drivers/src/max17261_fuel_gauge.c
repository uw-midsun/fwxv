#include "max17261_fuel_gauge.h"

#define PCT_LSB (1.0 / 256)  // LSBit is 1/256%
#define TIM_LSB (5625U)      // LSBit is 5625ms
#define CAP_LSB \
  (5000000.0 / storage->settings.r_sense_uohms)  // LSBit is 5 micro Volt hrs / Rsense
                                                 // Which is 5000000 pico Volt hrs / micro Rsense

StatusCode max17261_get_reg(Max17261Storage *storage, Max17261Registers reg, uint16_t *value) {
  status_ok_or_return(i2c_read_reg(storage->settings.i2c_port, storage->settings.i2c_address, reg,
                                   (uint8_t *)&value, sizeof(uint16_t)));
  return STATUS_CODE_OK;
}

StatusCode max17261_set_reg(Max17261Storage *storage, Max17261Registers reg, uint16_t value) {
  uint8_t buf[3];
  buf[0] = reg;
  // send LSByte then MSByte as per datasheet
  buf[1] = value & 0x00FF;
  buf[2] = value >> 8;
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, buf, sizeof(buf)));
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

StatusCode max17261_init(Max17261Storage *storage, Max17261Settings settings) {
  if (settings.i2c_port >= NUM_I2C_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->settings = settings;

  status_ok_or_return(max17261_set_reg(storage, MAX17261_DESIGN_CAP, settings.design_capacity));
  status_ok_or_return(max17261_set_reg(storage, MAX17261_I_CHG_TERM, settings.charge_term_current));
  uint16_t old_vempty = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_V_EMPTY, &old_vempty));
  // the 7 LSBits of the vempty reg is the recovery voltage, the last 9 are the empty voltage
  uint16_t new_vempty = (settings.empty_voltage << 7) + (old_vempty & 0x7F);
  status_ok_or_return(max17261_set_reg(storage, MAX17261_V_EMPTY, new_vempty));

  return STATUS_CODE_OK;
}
