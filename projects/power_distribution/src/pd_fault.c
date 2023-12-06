#include "pd_fault.h"

#include "adc.h"
#include "gpio.h"
#include "pd_hw.h"
#include "power_distribution_setters.h"

static uint8_t fault_bitset = 0;

// Fault threshold values
#define AUX_OVERCURRENT_THRESHOLD 8000   // Assuming ADC reading is in mA
#define DCDC_OVERCURRENT_THRESHOLD 8000  // Assuming ADC reading is in mA
#define AUX_OVERTEMP_THRESHOLD 60
#define DCDC_OVERTEMP_THRESHOLD 60

StatusCode prv_check_aux_fault() {
  const GpioAddress aux_fault_gpio_1 = AUX_FAULT_GPIO_1;
  const GpioAddress aux_fault_gpio_2 = AUX_FAULT_GPIO_2;

  GpioState aux_gpio_1_state;
  GpioState aux_gpio_2_state;
  status_ok_or_return(gpio_get_state(&aux_fault_gpio_1, &aux_gpio_1_state));
  status_ok_or_return(gpio_get_state(&aux_fault_gpio_2, &aux_gpio_2_state));

  if (!(fault_bitset & 0x1) &&
      (aux_gpio_1_state == GPIO_STATE_LOW || aux_gpio_2_state == GPIO_STATE_LOW)) {
    fault_bitset |= 0x1;
  } else if (fault_bitset & 0x1 && aux_gpio_1_state == GPIO_STATE_HIGH &&
             aux_gpio_2_state == GPIO_STATE_HIGH) {
    fault_bitset &= ~0x1;
  }

  return STATUS_CODE_OK;
}

StatusCode prv_check_dcdc_fault() {
  const GpioAddress dcdc_fault_gpio_1 = DCDC_FAULT_GPIO_1;
  const GpioAddress dcdc_fault_gpio_2 = DCDC_FAULT_GPIO_2;
  const GpioAddress dcdc_fault_gpio_3 = DCDC_FAULT_GPIO_3;

  GpioState dcdc_gpio_1_state;
  GpioState dcdc_gpio_2_state;
  GpioState dcdc_gpio_3_state;
  status_ok_or_return(gpio_get_state(&dcdc_fault_gpio_1, &dcdc_gpio_1_state));
  status_ok_or_return(gpio_get_state(&dcdc_fault_gpio_2, &dcdc_gpio_2_state));
  status_ok_or_return(gpio_get_state(&dcdc_fault_gpio_3, &dcdc_gpio_3_state));

  if (!(fault_bitset & 0x2) &&
      (dcdc_gpio_1_state == GPIO_STATE_LOW || dcdc_gpio_2_state == GPIO_STATE_LOW ||
       dcdc_gpio_3_state == GPIO_STATE_HIGH)) {
    fault_bitset |= 0x2;
  } else if (fault_bitset & 0x2 && dcdc_gpio_1_state == GPIO_STATE_HIGH &&
             dcdc_gpio_2_state == GPIO_STATE_HIGH && dcdc_gpio_3_state == GPIO_STATE_LOW) {
    fault_bitset &= ~0x2;
  }

  return STATUS_CODE_OK;
}

StatusCode prv_check_aux_overcurrent() {
  const GpioAddress aux_overcurrent = AUX_OVERCURRENT;

  uint16_t aux_current_reading;
  status_ok_or_return(adc_read_raw(aux_overcurrent, &aux_current_reading));

  if (!(fault_bitset & 0x4) && aux_current_reading > AUX_OVERCURRENT_THRESHOLD) {
    fault_bitset |= 0x4;
  } else if (fault_bitset & 0x4 && aux_current_reading <= AUX_OVERCURRENT_THRESHOLD) {
    fault_bitset &= ~0x4;
  }

  return STATUS_CODE_OK;
}

StatusCode prv_check_dcdc_overcurrent() {
  const GpioAddress dcdc_overcurrent = DCDC_OVERCURRENT;

  uint16_t dcdc_current_reading;
  status_ok_or_return(adc_read_raw(dcdc_overcurrent, &dcdc_current_reading));

  if (!(fault_bitset & 0x8) && dcdc_current_reading > DCDC_OVERCURRENT_THRESHOLD) {
    fault_bitset |= 0x8;
  } else if (fault_bitset & 0x8 && dcdc_current_reading <= DCDC_OVERCURRENT_THRESHOLD) {
    fault_bitset &= ~0x8;
  }

  return STATUS_CODE_OK;
}

StatusCode prv_check_aux_overtemp() {
  const GpioAddress aux_overtemp = AUX_OVERTEMP;

  uint16_t aux_temp_reading;
  status_ok_or_return(adc_read_converted(aux_overtemp, &aux_temp_reading));
  double aux_temp = (double)aux_temp_reading / 1000;

  if (!(fault_bitset & 0x10) && aux_temp > AUX_OVERTEMP_THRESHOLD) {
    fault_bitset |= 0x10;
  } else if (fault_bitset & 0x10 && aux_temp <= AUX_OVERTEMP_THRESHOLD) {
    fault_bitset &= ~0x10;
  }

  return STATUS_CODE_OK;
}

StatusCode prv_check_dcdc_overtemp() {
  const GpioAddress dcdc_overtemp = DCDC_OVERTEMP;

  uint16_t dcdc_temp_reading;
  status_ok_or_return(adc_read_converted(dcdc_overtemp, &dcdc_temp_reading));
  double dcdc_temp = (double)dcdc_temp_reading / 1000;

  if (!(fault_bitset & 0x20) && dcdc_temp_reading > DCDC_OVERTEMP_THRESHOLD) {
    fault_bitset |= 0x20;
  } else if (fault_bitset & 0x20 && dcdc_temp_reading <= DCDC_OVERTEMP_THRESHOLD) {
    fault_bitset &= ~0x20;
  }

  return STATUS_CODE_OK;
}

StatusCode check_pd_fault() {
  status_ok_or_return(prv_check_aux_fault());
  status_ok_or_return(prv_check_dcdc_fault());
  status_ok_or_return(prv_check_aux_overcurrent());
  status_ok_or_return(prv_check_dcdc_overcurrent());
  status_ok_or_return(prv_check_aux_overtemp());
  status_ok_or_return(prv_check_dcdc_overtemp());

  set_pd_status_fault_bitset(fault_bitset);

  return STATUS_CODE_OK;
}
