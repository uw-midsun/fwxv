#include "pd_fault.h"

#include "adc.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "outputs.h"
#include "pin_defs.h"
#include "power_distribution_getters.h"
#include "power_distribution_setters.h"
#include "soft_timer.h"

static uint8_t s_fault_bitset = 0;
static OutputState bps_light_state = OUTPUT_STATE_OFF;
static SoftTimer bps_timer;

// Fault pin address definitions
static const GpioAddress aux_valid1 = AUX_VALID1;
static const GpioAddress aux_valid2 = AUX_VALID2;
static const GpioAddress dcdc_valid1 = DCDC_VALID1;
static const GpioAddress dcdc_valid2 = DCDC_VALID2;

static void prv_set_fault_bit(uint8_t mask, bool condition) {
  if (condition) {
    s_fault_bitset |= mask;
  } else {
    s_fault_bitset &= ~(mask);
  }
}

bool check_aux_fault(void) {
  GpioState aux_valid1_state;
  GpioState aux_valid2_state;
  if (!status_ok(gpio_get_state(&aux_valid1, &aux_valid1_state)) ||
      !status_ok(gpio_get_state(&aux_valid2, &aux_valid2_state))) {
    return true;
  }
  return (aux_valid1_state == GPIO_STATE_HIGH || aux_valid2_state == GPIO_STATE_HIGH);
}

static bool prv_check_dcdc_fault(void) {
  GpioState dcdc_valid1_state;
  GpioState dcdc_valid2_state;
  if (!status_ok(gpio_get_state(&dcdc_valid1, &dcdc_valid1_state)) ||
      !status_ok(gpio_get_state(&dcdc_valid2, &dcdc_valid2_state))) {
    return true;
  }
  return (dcdc_valid1_state == GPIO_STATE_HIGH || dcdc_valid2_state == GPIO_STATE_HIGH);
}

static void prv_lights_bps_fault(SoftTimerId id) {
  if (get_battery_status_fault() || g_tx_struct.pd_status_bps_persist) {
    bps_light_state ^= 1;
    pd_set_output_group(OUTPUT_GROUP_BPS, bps_light_state);
  } else {
    bps_light_state = 0;
    pd_set_output_group(OUTPUT_GROUP_BPS, bps_light_state);
  }
  soft_timer_start(&bps_timer);
}

uint8_t check_pd_fault(void) {
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_AUX_FAULT_MASK, check_aux_fault());
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_DCDC_FAULT_MASK, prv_check_dcdc_fault());
  set_pd_status_fault_bitset(s_fault_bitset);
  return s_fault_bitset;
}

StatusCode init_bps_fault(void) {
  soft_timer_init_and_start(BPS_FAULT_LIGHT_PERIOD_MS, prv_lights_bps_fault, &bps_timer);
  return STATUS_CODE_OK;
}
