#include "adc.h"

#include <stddef.h>

#include "FreeRTOS.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// x86 implementation very similar to STM32F0 implementation.
// adc_read_raw should always return 2500.
// Vdda locked at 3300 mV.
// adc_read_converted should always return close to 2V
// temperature reading always returns 293 kelvin.

#define ADC_RETURNED_VOLTAGE_RAW 2500
#define ADC_CONTINUOUS_CB_FREQ_MS 50
#define ADC_TEMP_RETURN 293
#define ADC_VDDA_RETURN 3300

enum {
  ADC_CHANNEL_BAT = 0,
  ADC_CHANNEL_REF,
  ADC_CHANNEL_TEMP,
  ADC_SPECIAL_CHANNELS,
};

const GpioAddress ADC_BAT = {
  .port = NUM_GPIO_PORTS,
  .pin = ADC_CHANNEL_BAT,
};
const GpioAddress ADC_REF = {
  .port = NUM_GPIO_PORTS,
  .pin = ADC_CHANNEL_REF,
};
const GpioAddress ADC_TEMP = {
  .port = NUM_GPIO_PORTS,
  .pin = ADC_CHANNEL_TEMP,
};

#define NUM_ADC_CHANNELS 18

static uint16_t s_adc_readings[NUM_ADC_CHANNELS];

static bool s_active_channels[NUM_ADC_CHANNELS];

static uint16_t prv_get_temp(uint16_t reading) {
  return ADC_TEMP_RETURN;
}

static uint16_t prv_get_vdda(uint16_t reading) {
  return ADC_VDDA_RETURN;
}

static StatusCode prv_check_channel_valid_and_enabled(AdcChannel adc_channel) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (!s_active_channels[adc_channel]) {
    return status_code(STATUS_CODE_EMPTY);
  }
  return STATUS_CODE_OK;
}

static void prv_periodic_continous_cb(SoftTimerId id, void *context) {
  for (AdcChannel i = 0; i < NUM_ADC_CHANNELS; i++) {
    if (s_adc_interrupts[i].callback != NULL) {
      s_adc_interrupts[i].callback(i, s_adc_interrupts[i].context);
    } else if (s_adc_interrupts[i].pin_callback != NULL) {
      s_adc_interrupts[i].pin_callback(prv_channel_to_gpio(i), s_adc_interrupts[i].context);
    }
  }
  soft_timer_start_millis(ADC_CONTINUOUS_CB_FREQ_MS, prv_periodic_continous_cb, NULL, NULL);
}

void adc_init(AdcMode adc_mode) {
  if (adc_mode == ADC_MODE_CONTINUOUS) {
    soft_timer_start_millis(ADC_CONTINUOUS_CB_FREQ_MS, prv_periodic_continous_cb, NULL, NULL);
  }
  for (size_t i = 0; i < NUM_ADC_CHANNELS; ++i) {
    prv_reset_channel(i);
  }
  adc_set_channel(ADC_REF, true);
}

// ADC Channel to GPIO Address mapping found in table 13 of the specific device
// datasheet. Channels 0 to 7 are occupied by port A, 8 to 9 by prt B, and 10 to
// 15 by port C
StatusCode adc_get_channel(GpioAddress address, uint8_t *adc_channel) {
  *adc_channel = address.pin;
  switch (address.port) {
    case GPIO_PORT_A:
      if (address.pin > 7) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      *adc_channel += ADC_SPECIAL_CHANNELS;
      break;
    case GPIO_PORT_B:
      if (address.pin > 1) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      *adc_channel += ADC_SPECIAL_CHANNELS + 8;
      break;
    case GPIO_PORT_C:
      if (address.pin > 5) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      *adc_channel += ADC_SPECIAL_CHANNELS + 10;
      break;
    case NUM_GPIO_PORTS:  // for special channels BAT/REF/TEMP
      if (address.pin >= ADC_SPECIAL_CHANNELS) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      break;
    default:
      return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (*adc_channel > NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return STATUS_CODE_OK;
}

// the following functions are wrappers over the legacy AdcChannel API dealing with GpioAddresses
// instead
StatusCode adc_set_channel(GpioAddress address, bool new_state) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  s_active_channels[channel] = new_state;
}

StatusCode adc_read_raw(GpioAddress address, uint16_t *reading) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));

  s_adc_readings[channel] = ADC_RETURNED_VOLTAGE_RAW;
  *reading = s_adc_readings[channel];

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(GpioAddress address, uint16_t *reading) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));

  uint16_t adc_reading = 0;
  status_ok_or_return(adc_read_raw(address, &adc_reading));

  switch (channel) {
    case ADC_CHANNEL_TEMP:
      *reading = prv_get_temp(adc_reading);
      return STATUS_CODE_OK;
    case ADC_CHANNEL_REF:
      *reading = prv_get_vdda(adc_reading);
      return STATUS_CODE_OK;
    case ADC_CHANNEL_BAT:
      adc_reading *= 2;
      break;
    default:
      break;
  }
  uint16_t vdda;
  adc_read_converted(ADC_REF, &vdda);
  *reading = (adc_reading * vdda) / 4095;
  return STATUS_CODE_OK;
}
