#include "pd_fault.h"

#include "adc.h"
#include "exported_enums.h"
#include "gpio.h"
#include "pin_defs.h"
#include "power_distribution_setters.h"

static uint8_t s_fault_bitset = 0;

// Fault pin address definitions
static const GpioAddress aux_fault_gpio_1 = AUX_FAULT_GPIO_1;
static const GpioAddress aux_fault_gpio_2 = AUX_FAULT_GPIO_2;
static const GpioAddress dcdc_fault_gpio_1 = DCDC_FAULT_GPIO_1;
static const GpioAddress dcdc_fault_gpio_2 = DCDC_FAULT_GPIO_2;
static const GpioAddress dcdc_fault_gpio_3 = DCDC_FAULT_GPIO_3;

static void prv_set_fault_bit(uint8_t mask, bool condition) {
  if (condition) {
    s_fault_bitset |= mask;
  } else {
    s_fault_bitset &= ~(mask);
  }
}

static StatusCode prv_check_aux_fault(void) {
  GpioState aux_gpio_1_state;
  GpioState aux_gpio_2_state;
  status_ok_or_return(gpio_get_state(&aux_fault_gpio_1, &aux_gpio_1_state));
  status_ok_or_return(gpio_get_state(&aux_fault_gpio_2, &aux_gpio_2_state));

  bool aux_fault = (aux_gpio_1_state == GPIO_STATE_LOW || aux_gpio_2_state == GPIO_STATE_LOW);
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_AUX_FAULT_MASK, aux_fault);

  return STATUS_CODE_OK;
}

static StatusCode prv_check_dcdc_fault(void) {
  GpioState dcdc_gpio_1_state;
  GpioState dcdc_gpio_2_state;
  GpioState dcdc_gpio_3_state;
  status_ok_or_return(gpio_get_state(&dcdc_fault_gpio_1, &dcdc_gpio_1_state));
  status_ok_or_return(gpio_get_state(&dcdc_fault_gpio_2, &dcdc_gpio_2_state));
  status_ok_or_return(gpio_get_state(&dcdc_fault_gpio_3, &dcdc_gpio_3_state));

  bool dcdc_fault = (dcdc_gpio_1_state == GPIO_STATE_LOW || dcdc_gpio_2_state == GPIO_STATE_LOW ||
                     dcdc_gpio_3_state == GPIO_STATE_HIGH);
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_DCDC_FAULT_MASK, dcdc_fault);

  return STATUS_CODE_OK;
}

StatusCode check_pd_fault() {
  status_ok_or_return(prv_check_aux_fault());
  status_ok_or_return(prv_check_dcdc_fault());

  set_pd_status_fault_bitset(s_fault_bitset);

  return STATUS_CODE_OK;
}