#include "max17261_fuel_gauge.h"

#include <inttypes.h>

#include "log.h"
#include "delay.h"

// See Table 3 on pg.18 of the datasheet
#define PCT_LSB (1.0f / 256)                                  // (%)   LSBit is 1/256%
#define CAP_LSB (5.0f / storage->settings->sense_resistor_mohms)     // (mAh) LSBit is 5 mili Volt hrs / Rsense (mAh)
#define TIM_LSB (5625U)                                       // (ms)  LSBit is 5625ms
#define CUR_LSB (1.5625f / storage->settings->sense_resistor_mohms)  // (mA)  LSBit is 1.5625uA / Rsense
#define VOLT_LSB (1.25f / 16)                                 // (mV)  LSBit is 1.25mV / 16
#define TEMP_LSB (1.0f / 256)                                 // (C)   LSBit is 1 / 256 C

static StatusCode max17261_get_reg(Max17261Storage *storage, Max17261Registers reg,
                                   uint16_t *value) {
  // unsure of underlying type of enum, cast to uint8_t to be sure
  uint8_t reg8 = reg;
  status_ok_or_return(i2c_write(storage->settings->i2c_port, storage->settings->i2c_address, &reg8,
                                sizeof(uint8_t)));
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
  // clear lock bit if set - set on state of charge change
  uint16_t reg = 0;
  max17261_get_reg(storage, MAX17261_STATUS, &reg);
  if (reg & (1 << 7)) {
    max17261_set_reg(storage, MAX17261_STATUS, reg & (uint16_t)~(1 << 7));
  }

  uint16_t status = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_STATUS, &status));

  uint16_t soc_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_SOC, &soc_reg_val));
  *soc_pct = soc_reg_val * PCT_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_remaining_capacity(Max17261Storage *storage, uint32_t *rem_cap_mAh) {
  uint16_t rem_cap_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_CAP, &rem_cap_reg_val));
  *rem_cap_mAh = rem_cap_reg_val * CAP_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_full_capacity(Max17261Storage *storage, uint32_t *full_cap_mAh) {
  uint16_t full_cap_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_FULL_CAP_REP, &full_cap_reg_val));
  *full_cap_mAh = full_cap_reg_val * CAP_LSB;
  
  uint16_t design = 0;
  max17261_get_reg(storage, MAX17261_DESIGN_CAP, &design);
  LOG_DEBUG("DESIGNCAP: %" PRIu32 "mAh\n", (uint32_t)((uint32_t)design * CAP_LSB));

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

StatusCode max17261_current(Max17261Storage *storage, int16_t *current_ua) {
  uint16_t current_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_CURRENT, &current_reg_val));
  *current_ua = (int16_t)(current_reg_val)*CUR_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_voltage(Max17261Storage *storage, uint16_t *vcell_mv) {
  uint16_t vcell_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_VCELL, &vcell_reg_val));
  *vcell_mv = (uint16_t)((float)(vcell_reg_val) * VOLT_LSB);

  uint16_t vempty = 0; // TODO remove
  max17261_get_reg(storage, MAX17261_V_EMPTY, &vempty);
  LOG_DEBUG("VEMPTY: %dmV\n", (int) ((float)vempty * VOLT_LSB));
  return STATUS_CODE_OK;
}

StatusCode max17261_temp(Max17261Storage *storage, uint16_t *temp_c) {
  uint16_t temp_reg_val = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_TEMP, &temp_reg_val));
  *temp_c = temp_reg_val * TEMP_LSB;
  return STATUS_CODE_OK;
}

StatusCode max17261_init(Max17261Storage *storage, Max17261Settings *settings) {
  if (settings->i2c_port >= NUM_I2C_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->settings = settings;

  // ** Based on https://web.archive.org/web/20240330212616/https://pdfserv.maximintegrated.com/en/an/MAX1726x-Software-Implementation-user-guide.pdf
  // Step 0 - check if already configured
  uint16_t status = 0;
  status_ok_or_return(max17261_get_reg(storage, MAX17261_STATUS, &status));
  if ((status & 0x0002) != 0) {

    // Step 1 -- delay until FSTAT.DNR bit == 0
    uint16_t fstat = 0;
    status_ok_or_return(max17261_get_reg(storage, MAX17261_FSTAT, &fstat));
    while ((fstat & 1) != 0) {
      LOG_DEBUG("data not ready, fstat: %d (%d)\n", fstat, fstat & 1);
      status_ok_or_return(max17261_get_reg(storage, MAX17261_FSTAT, &fstat));

      delay_ms(10);
    }

    // Step 2 -- disable hibernation
    uint16_t hibcfg = 0;
    status_ok_or_return(max17261_get_reg(storage, MAX17261_HIB_CFG, &hibcfg));
    status_ok_or_return(max17261_set_reg(storage, MAX17261_SOFT_WAKEUP, 0x90)); // wakup ic
    status_ok_or_return(max17261_set_reg(storage, MAX17261_HIB_CFG, 0x0));  // disable hibernation
    status_ok_or_return(max17261_set_reg(storage, MAX17261_SOFT_WAKEUP, 0x0));  // clear wakeup command

    // Step 2.1 -- configure
    status_ok_or_return(max17261_set_reg(storage, MAX17261_DESIGN_CAP, settings->pack_design_cap_mah / (uint32_t) CAP_LSB));
    status_ok_or_return(max17261_set_reg(storage, MAX17261_I_CHG_TERM, settings->charge_term_current_ma / CUR_LSB));
    status_ok_or_return(max17261_set_reg(storage, MAX17261_V_EMPTY, settings->cell_empty_voltage_v / VOLT_LSB));

    status_ok_or_return(max17261_set_reg(storage, MAX17261_SOC_HOLD, 0x0)); // disable SOCHold, not relevant to us

    uint16_t modelcfg = /*refresh*/ (1 << 15) | /*R100*/ (0 << 13) | /*RChg*/ (0 << 10) | (1 << 3);
    status_ok_or_return(max17261_set_reg(storage, MAX17261_MODEL_I_CFG, modelcfg));

    // wait for modelcfg refresh to complete
    while (modelcfg & (1 << 15)) {
      LOG_DEBUG("modelcfg refresh not cleared: %d\n", modelcfg);
      status_ok_or_return(max17261_get_reg(storage, MAX17261_MODEL_I_CFG, &modelcfg));
      delay_ms(10);
    }

    // enable hibernation
    //status_ok_or_return(max17261_set_reg(storage, MAX17261_HIB_CFG, hibcfg));    
  }

  // Step 3
  status_ok_or_return(max17261_get_reg(storage, MAX17261_STATUS, &status));
  status_ok_or_return(max17261_set_reg(storage, MAX17261_STATUS, status & (uint16_t)~(1 << 1))); // clear status POR bit

  LOG_DEBUG("ok\n");
  uint16_t cap = 0;

  max17261_get_reg(storage, MAX17261_DESIGN_CAP, &cap);
  LOG_DEBUG("design cap: %d - %d - %d - %d\n", cap, (int) ((float)((float)cap * (float) CAP_LSB)), (uint16_t) settings->pack_design_cap_mah, (int) (settings->pack_design_cap_mah / CAP_LSB));

  delay_ms(5000);
  return STATUS_CODE_OK;
}
