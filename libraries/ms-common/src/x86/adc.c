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

#define ADC_PRIORITY 1

enum {
  ADC_CHANNEL_BAT = 16,
  ADC_CHANNEL_REF,
  ADC_CHANNEL_TEMP,
  NUM_ADC_CHANNELS,
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

typedef struct AdcStore {
  uint16_t reading;
  Task *task;
  Event event;
} AdcStore;

static AdcStore s_adc_stores[NUM_ADC_CHANNELS];

static bool s_active_channels[NUM_ADC_CHANNELS];

static uint16_t prv_get_temp(uint16_t reading) {
  return ADC_TEMP_RETURN;
}

static uint16_t prv_get_vdda(uint16_t reading) {
  return ADC_VDDA_RETURN;
}

static StatusCode prv_check_channel_enabled(uint8_t adc_channel) {
  if (!s_active_channels[adc_channel]) {
    return status_code(STATUS_CODE_EMPTY);
  }
  return STATUS_CODE_OK;
}

TASK(periodic_callback, TASK_STACK_256) {
  TickType_t last_wake_time = xTaskGetTickCount();

  while (true) {
    xTaskDelayUntil(&last_wake_time, ADC_CONTINUOUS_CB_FREQ_MS);

    for (uint8_t i = 0; i < NUM_ADC_CHANNELS; ++i) {
      if (s_active_channels[i] && s_adc_stores[i].task != NULL) {
        notify(s_adc_stores[i].task->handle, s_adc_stores[i].event);
      }
    }
  }
}

void adc_init(AdcMode adc_mode) {
  if (adc_mode == ADC_MODE_CONTINUOUS && periodic_callback->context == NULL) {
    tasks_init_task(periodic_callback, ADC_PRIORITY, (int *)1);
  }
  for (size_t i = 0; i < NUM_ADC_CHANNELS; ++i) {
    s_adc_stores[i].reading = 0;
    s_adc_stores[i].task = NULL;
    s_adc_stores[i].event = 0;
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
      break;
    case GPIO_PORT_B:
      if (address.pin > 1) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      *adc_channel += 8;
      break;
    case GPIO_PORT_C:
      if (address.pin > 5) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      *adc_channel += 10;
      break;
    case NUM_GPIO_PORTS:  // for special channels BAT/REF/TEMP
      if (address.pin < ADC_CHANNEL_BAT || address.pin > ADC_CHANNEL_TEMP) {
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

StatusCode adc_register_event(GpioAddress address, Task *task, Event event) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));
  s_adc_stores[channel].task = task;
  s_adc_stores[channel].event = event;
  return STATUS_CODE_OK;
}

// the following functions are wrappers over the legacy AdcChannel API dealing with GpioAddresses
// instead
StatusCode adc_set_channel(GpioAddress address, bool new_state) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  s_active_channels[channel] = new_state;
  return STATUS_CODE_OK;
}

StatusCode adc_read_raw(GpioAddress address, uint16_t *reading) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));

  s_adc_stores[channel].reading = ADC_RETURNED_VOLTAGE_RAW;
  *reading = s_adc_stores[channel].reading;

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
