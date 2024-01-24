#include "adc.h"

#include <stddef.h>
#include <string.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "semaphore.h"
#include "soft_timer.h"

#define ADC_Channel_Vrefint 17
#define ADC_Channel_TempSensor 16
#define MOCK_VREFINT 1489
#define MAX_ADC_READINGS 16
#define VREFINT_MV 1200

typedef struct AdcStatus {
  bool initialized;
  uint8_t active_channels;  // Keeps track of how many channels have been registered with the ADC
  Semaphore converting;
} AdcStatus;

// Adc1 peripheral status
static AdcStatus s_adc_status;

// Array of readings, in order of rank, which DMA will write to
static volatile uint16_t s_adc_readings[MAX_ADC_READINGS];

// Store of ranks for each channel, at their channel index
static uint8_t s_adc_ranks[NUM_ADC_CHANNELS];

// Mock Gpio addresses for internal channels
const GpioAddress ADC_TEMP = {
  .port = NUM_GPIO_PORTS,
  .pin = ADC_Channel_TempSensor,  // Channel 16
};
const GpioAddress ADC_REF = {
  .port = NUM_GPIO_PORTS,
  .pin = ADC_Channel_Vrefint,  // Channel 17
};

// Convert GPIO Address to one of 16 ADC channels Channels 0 to 7 are occupied
// by pins A0-A7, 8 to 9 by pins B0-B1, and 10 to 15 by pins C0-C5
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
      if (address.pin != ADC_Channel_Vrefint && address.pin != ADC_Channel_TempSensor) {
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

// Formula obtained from section 13.9 of the reference manual. Returns reading
// in kelvin
static uint16_t prv_get_temp(uint16_t reading) {
  return 2500;
}

// Checks if a channel has been registered
static StatusCode prv_check_channel_enabled(uint8_t channel) {
  if (s_adc_ranks[channel] != 0) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNINITIALIZED;
  }
}

// Registers a GPIO Address as an channel with the ADC
// Must be called before ADC initialized
StatusCode adc_add_channel(GpioAddress address) {
  if (s_adc_status.initialized) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Get channel associated with pin, check that it has not already been initialized
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  if (prv_check_channel_enabled(channel) == STATUS_CODE_OK) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Set rank for this channel index
  if (++s_adc_status.active_channels < NUM_ADC_CHANNELS) {
    s_adc_ranks[channel] = s_adc_status.active_channels;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

StatusCode adc_init(void) {
  if (s_adc_status.initialized) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (s_adc_status.active_channels == 0) {
    return STATUS_CODE_INVALID_ARGS;  // Need to have added channels
  }

  adc_add_channel(ADC_REF);

  s_adc_readings[s_adc_ranks[ADC_Channel_Vrefint] - 1] = MOCK_VREFINT;

  // Initialize static variables
  sem_init(&s_adc_status.converting, 1, 0);
  s_adc_status.initialized = true;
  return STATUS_CODE_OK;
}

void adc_deinit(void) {
  memset(&s_adc_status, 0, sizeof(s_adc_status));
  memset(s_adc_readings, 0, sizeof(uint16_t) * MAX_ADC_READINGS);
  memset(s_adc_ranks, 0, sizeof(uint8_t) * NUM_ADC_CHANNELS);
}

// Mimic ISR behaviour
static void prv_adc_mock_reading(void) {
  LOG_DEBUG("Reading ADC\n");
  delay_ms(100);
  mutex_unlock(&s_adc_status.converting);
}

StatusCode adc_run(void) {
  prv_adc_mock_reading();
  // Once conversion is finished, we will receive the semaphore from ISR
  status_ok_or_return(sem_wait(&s_adc_status.converting, ADC_TIMEOUT_MS));
  return STATUS_CODE_OK;
}

StatusCode adc_read_raw(GpioAddress address, uint16_t *reading) {
  if (!reading) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));
  // Index of reading is its rank - 1
  *reading = s_adc_readings[s_adc_ranks[channel] - 1];
  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(GpioAddress address, uint16_t *reading) {
  if (!reading) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));

  uint16_t adc_reading = s_adc_readings[s_adc_ranks[channel] - 1];
  uint16_t vref = s_adc_readings[s_adc_ranks[ADC_Channel_Vrefint] - 1];

  if (channel == ADC_Channel_TempSensor) {
    *reading = prv_get_temp(adc_reading);
    return STATUS_CODE_OK;
  }

  // Get latest vref value to convert read value to a voltage
  if (vref) {
    *reading = (adc_reading * VREFINT_MV) / vref;
  } else {
    *reading = 0;
  }
  return STATUS_CODE_OK;
}
StatusCode adc_set_reading(GpioAddress sample_address, uint16_t adc_reading) {
  uint8_t adc_channel;
  status_ok_or_return(adc_get_channel(sample_address, &adc_channel));
  status_ok_or_return(prv_check_channel_enabled(adc_channel));
  // We will convert our passed value in mV to a raw reading
  s_adc_readings[s_adc_ranks[adc_channel] - 1] = (adc_reading * MOCK_VREFINT) / VREFINT_MV;
  return STATUS_CODE_OK;
}
