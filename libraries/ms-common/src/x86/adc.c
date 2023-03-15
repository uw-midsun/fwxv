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

typedef struct AdcStatus {
  bool initialized;
  uint8_t active_channels;    // Keeps track of how many channels have been registered with the ADC
  volatile uint8_t sequence;  // Indicates where next conversion will go
  bool continuous;            // Determines whether conversions are continuous or single-shot
  Mutex converting;
} AdcStatus;

typedef struct AdcStore {
  uint8_t channel;
  uint16_t reading;
} AdcStore;

// Adc1 peripheral status
static AdcStatus s_adc_status;

// Store of information about each possible channel
// Channels are stored at their relative index
// An AdcStore array is maintained to determine the order in which the adc_values are stored
static AdcStore s_adc_stores[NUM_ADC_CHANNELS];
static AdcStore *s_adc_sequence[NUM_ADC_CHANNELS];

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
      if (address.pin != ADC_Channel_Vrefint || address.pin != ADC_Channel_TempSensor) {
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
  return 25;
}

// Formula obtained from section 13.9 of the reference manual. Returns Vdda in
// mV
static uint16_t prv_get_vdda(uint16_t reading) {
  // To avoid dividing by zero faults:
  if (!reading) {
    return reading;
  }
  return 3300;
}

// Checks if a channel has been registered
static StatusCode prv_check_channel_enabled(uint8_t channel) {
  bool found = false;
  // Check each index in the sequence to see if it has a matching channel
  for (uint8_t index = 0; index < s_adc_status.active_channels; index++) {
    if (s_adc_sequence[index] == &s_adc_stores[channel]) {
      found = true;
      break;
    }
  }
  if (found) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNINITIALIZED;
  }
}

// Registers a GPIO Address as an channel with the ADC
// Must be called before ADC initialized
StatusCode adc_add_channel(GpioAddress address) {
  if (s_adc_status.initialized) {
    LOG_DEBUG("Channel already initialized\n");
    return status_msg(STATUS_CODE_INVALID_ARGS,
                      "Channels must be initialized before adc_init called");
  }

  // Get channel associated with pin, check that it has not already been initialized
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  if (prv_check_channel_enabled(channel) == STATUS_CODE_OK) {
    LOG_DEBUG("Channel already initialized\n");
    return STATUS_CODE_INVALID_ARGS;
  }

  // Store address of channel at incrementing sequence entries
  // This is needed as for each adc iteration, it will convert all the channels
  // in the order that they are registered
  if (s_adc_status.active_channels < NUM_ADC_CHANNELS) {
    s_adc_sequence[s_adc_status.active_channels] = &s_adc_stores[channel];
    s_adc_status.active_channels++;
  } else {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Number of possible channels exceeded");
  }
  return STATUS_CODE_OK;
}

StatusCode adc_init(AdcMode adc_mode) {
  if (s_adc_status.initialized) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "adc_init should only be called once");
  }
  if (s_adc_status.active_channels == 0) {
    return STATUS_CODE_INVALID_ARGS;  // Need to have added channels
  }

  // Initialize each adc store with it's own channel for easy initialization
  for (uint8_t channel = 0; channel < NUM_ADC_CHANNELS; channel++) {
    s_adc_stores[channel].channel = channel;
  }

  // Initialize all channels registered via adc_add_channel()
  for (uint8_t index = 0; index < s_adc_status.active_channels; index++) {
    // Each channel is configured with ascending rank, starting at one
    uint8_t rank = index + 1;
  }

  // By default, enable vref and temp sensor for voltage conversions
  adc_add_channel(ADC_REF);

  // Initialize static variables
  s_adc_status.continuous = adc_mode;
  s_adc_status.sequence = 0;
  mutex_init(&s_adc_status.converting);
  s_adc_status.initialized = true;

  return STATUS_CODE_OK;
}

void adc_deinit(void) {
  memset(&s_adc_status, 0, sizeof(s_adc_status));
  memset(s_adc_stores, 0, sizeof(AdcStore) * NUM_ADC_CHANNELS);
  memset(s_adc_sequence, 0, sizeof(AdcStore *) * NUM_ADC_CHANNELS);
}

static void adc_mock(uint8_t channel) {
  // Use hardcoded value for now
  LOG_DEBUG("Reading ADC\n");
  s_adc_stores[channel].reading = 0xAA;
  s_adc_stores[ADC_Channel_Vrefint].reading = 0x1;
  delay_ms(100);
  mutex_unlock(&s_adc_status.converting);
}

StatusCode adc_read_raw(GpioAddress address, uint16_t *reading) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));

  if (!s_adc_status.continuous) {
    // For Single-shot, we take semaphore and initiate a conversion
    mutex_lock(&s_adc_status.converting, ADC_TIMEOUT_MS);

    adc_mock(channel);
    // Once conversion is finished, we will receive the semaphore from ISR
    mutex_lock(&s_adc_status.converting, ADC_TIMEOUT_MS);
    mutex_unlock(&s_adc_status.converting);
  }

  *reading = s_adc_stores[channel].reading;

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(GpioAddress address, uint16_t *reading) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));

  uint16_t adc_reading = 0;
  status_ok_or_return(adc_read_raw(address, &adc_reading));
  LOG_DEBUG("READING: %d\n", adc_reading);

  switch (channel) {
    case ADC_Channel_TempSensor:
      *reading = prv_get_temp(adc_reading);
      return STATUS_CODE_OK;
    case ADC_Channel_Vrefint:
      *reading = prv_get_vdda(adc_reading);
      return STATUS_CODE_OK;
    default:
      break;
  }
  // Get latest vref value to convert read value to a voltage
  uint16_t vdda = s_adc_stores[ADC_Channel_Vrefint].reading;
  *reading = (adc_reading * vdda) / 4095;
  return STATUS_CODE_OK;
}

// Getters and setters to test if ADC works
void set_reading(GpioAddress sample_address, uint16_t *adc_reading) {
  uint8_t adc_channel;
  gpio_init_pin(&sample_address, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  adc_add_channel(sample_address);
  adc_init(ADC_MODE_SINGLE);
  adc_get_channel(sample_address, &adc_channel);
  // This should mimic what adc_mock would to be doing
  s_adc_stores[adc_channel].channel = adc_channel;
  s_adc_stores[adc_channel].reading = *adc_reading;
  s_adc_stores[ADC_Channel_Vrefint].reading = 4095;
  delay_ms(20);
  mutex_unlock(&s_adc_status.converting);
}
uint16_t get_reading(GpioAddress sample_address) {
  uint8_t adc_channel;
  adc_get_channel(sample_address, &adc_channel);
  return s_adc_stores[adc_channel].reading;
}
