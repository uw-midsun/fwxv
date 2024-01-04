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
static const GpioAddress aux_overcurrent = AUX_OVERCURRENT;
static const GpioAddress dcdc_overcurrent = DCDC_OVERCURRENT;
static const GpioAddress aux_overtemp = AUX_OVERTEMP;
static const GpioAddress dcdc_overtemp = DCDC_OVERTEMP;

// Fault threshold values
#define AUX_OVERCURRENT_THRESHOLD 8000   // Assuming ADC reading is in mA
#define DCDC_OVERCURRENT_THRESHOLD 8000  // Assuming ADC reading is in mA
#define AUX_OVERTEMP_THRESHOLD 60
#define DCDC_OVERTEMP_THRESHOLD 60

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

static StatusCode prv_check_aux_overcurrent(void) {
  uint16_t aux_current_reading;
  status_ok_or_return(adc_read_raw(aux_overcurrent, &aux_current_reading));

  bool aux_overcurrent = (aux_current_reading > AUX_OVERCURRENT_THRESHOLD);
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_AUX_OVERCURRENT_MASK, aux_overcurrent);

  return STATUS_CODE_OK;
}

static StatusCode prv_check_dcdc_overcurrent(void) {
  uint16_t dcdc_current_reading;
  status_ok_or_return(adc_read_raw(dcdc_overcurrent, &dcdc_current_reading));

  bool dcdc_overcurrent = (dcdc_current_reading > DCDC_OVERCURRENT_THRESHOLD);
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_DCDC_OVERCURRENT_MASK, dcdc_overcurrent);

  return STATUS_CODE_OK;
}

static StatusCode prv_check_aux_overtemp(void) {
  uint16_t aux_temp_reading;
  status_ok_or_return(adc_read_converted(aux_overtemp, &aux_temp_reading));
  double aux_temp = (double)aux_temp_reading / 1000;

  bool aux_overtemp = (aux_temp > AUX_OVERTEMP_THRESHOLD);
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_AUX_OVERTEMP_MASK, aux_overtemp);

  return STATUS_CODE_OK;
}

static StatusCode prv_check_dcdc_overtemp(void) {
  uint16_t dcdc_temp_reading;
  status_ok_or_return(adc_read_converted(dcdc_overtemp, &dcdc_temp_reading));
  double dcdc_temp = (double)dcdc_temp_reading / 1000;

  bool dcdc_overtemp = (dcdc_temp_reading > DCDC_OVERTEMP_THRESHOLD);
  prv_set_fault_bit(EE_PD_STATUS_FAULT_BITSET_DCDC_OVERTEMP_MASK, dcdc_overtemp);

  return STATUS_CODE_OK;
}

StatusCode check_pd_fault() {
  status_ok_or_return(prv_check_aux_fault());
  status_ok_or_return(prv_check_dcdc_fault());
  status_ok_or_return(prv_check_aux_overcurrent());
  status_ok_or_return(prv_check_dcdc_overcurrent());
  status_ok_or_return(prv_check_aux_overtemp());
  status_ok_or_return(prv_check_dcdc_overtemp());

  set_pd_status_fault_bitset(s_fault_bitset);

  return STATUS_CODE_OK;
}
