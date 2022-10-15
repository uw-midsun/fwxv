#include "adc.h"

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "gpio.h"
#include "log.h"
#include "semphr.h"
#include "status.h"
#include "stm32f10x.h"
#include "stm32f10x_interrupt.h"

// Mock Gpio addresses for internal channels
const GpioAddress ADC_TEMP = {
  .port = NUM_GPIO_PORTS,
  .pin = ADC_Channel_TempSensor,  // Channel 16
};
const GpioAddress ADC_REF = {
  .port = NUM_GPIO_PORTS,
  .pin = ADC_Channel_Vrefint,  // Channel 17
};

// TS_CAL addresses obtained from section 3.10.1 of the specific device
// datasheet
// TODO(mitchellostler): Verify these calibration values are the same on F1
#define ADC_TS_CAL1 0x1FFFF7b8
#define ADC_TS_CAL2 0x1FFFF7c2

// ADC_VREFINT_CAL address obtained from section 3.10.2 of the specific device
// datasheet
#define ADC_VREFINT_CAL 0x1FFFF7ba

typedef struct AdcStatus {
  bool initialized;
  uint8_t active_channels;    // Keeps track of how many channels have been registered with the ADC
  volatile uint8_t sequence;  // Indicates where next conversion will go
  bool continuous;            // Determines whether conversions are continuous or single-shot
  SemaphoreHandle_t converting;
  StaticSemaphore_t converting_buf;
} AdcStatus;

static AdcStatus s_adc_status;

typedef struct AdcStore {
  uint8_t channel;
  uint16_t reading;
  Task *task;
  Event event;
} AdcStore;

// Store of information about each possible channel
// Channels are stored at their relative index
// An AdcStore array is maintained to determine the order in which the adc_values are stored
static AdcStore s_adc_stores[NUM_ADC_CHANNELS];
static AdcStore *s_adc_sequence[NUM_ADC_CHANNELS];

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
      if (address.pin < ADC_Channel_Vrefint || address.pin > ADC_Channel_TempSensor) {
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
  uint16_t ts_cal1 = *(uint16_t *)ADC_TS_CAL1;
  uint16_t ts_cal2 = *(uint16_t *)ADC_TS_CAL2;

  reading = ((110 - 30) * (reading - ts_cal1)) / (ts_cal2 - ts_cal1) + 30;

  return reading + 273;
}

// Formula obtained from section 13.9 of the reference manual. Returns Vdda in
// mV
static uint16_t prv_get_vdda(uint16_t reading) {
  // To avoid dividing by zero faults:
  if (!reading) {
    return reading;
  }
  uint16_t vrefint_cal = *(uint16_t *)ADC_VREFINT_CAL;
  reading = (3300 * vrefint_cal) / reading;
  return reading;
}

// Checks if a channel has been registered
static StatusCode prv_check_channel_enabled(uint8_t channel) {
  bool found = false;
  // Check each index in the sequence to see if it has a matching channel
  for (uint8_t index = 0; index < s_adc_status.active_channels; index++) {
    if (s_adc_sequence[index] == &s_adc_stores[channel]) {
      found = true;
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
    return status_msg(STATUS_CODE_INVALID_ARGS,
                      "Channels must be initialized before adc_init called");
  }

  // Get channel associated with pin, check that it has not already been initialized
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  if (prv_check_channel_enabled(channel) == STATUS_CODE_OK) {
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

StatusCode adc_register_event(GpioAddress address, Task *task, Event event) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));
  s_adc_stores[channel].task = task;
  s_adc_stores[channel].event = event;
  return STATUS_CODE_OK;
}

StatusCode adc_init(AdcMode adc_mode) {
  if (s_adc_status.initialized) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "adc_init should only be called once");
  }
  if (s_adc_status.active_channels == 0) {
    return STATUS_CODE_INVALID_ARGS;  // Need to have added channels
  }

  // Clear Previous settings
  ADC_DeInit(ADC1);

  // Enable ADC1 Clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, true);

  // Initialize each adc store with it's own channel for easy initialization
  for (uint8_t channel = 0; channel < NUM_ADC_CHANNELS; channel++) {
    s_adc_stores[channel].channel = channel;
  }

  // Initialize all channels registered via adc_add_channel()
  for (uint8_t index = 0; index < s_adc_status.active_channels; index++) {
    // Each channel is configured with ascending rank, starting at one
    uint8_t rank = index + 1;
    ADC_InjectedChannelConfig(ADC1, s_adc_sequence[index]->channel, rank, ADC_SAMPLE_RATE);
  }

  // Initialize ADC1
  ADC_InitTypeDef adc_settings = {
    .ADC_Mode = ADC_Mode_Independent,    // Use only one ADC
    .ADC_ScanConvMode = ENABLE,          // Use multi-channel scan
    .ADC_ContinuousConvMode = adc_mode,  // Continuous or one-shot
    .ADC_ExternalTrigConv =
        ADC_ExternalTrigConv_None,         // Don't need to trigger adc on external stimuli
    .ADC_DataAlign = ADC_DataAlign_Right,  // Use rightmost 12 bits of ADC register
    .ADC_NbrOfChannel = s_adc_status.active_channels,  // Scan as many channels as we've initialized
  };
  ADC_Init(ADC1, &adc_settings);

  // Enable the ADC
  ADC_Cmd(ADC1, true);

  // Calibrate ADC
  // Reset Calibration regs, wait for flag to be cleared
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1)) {
  }

  // Start and Wait for calibration to complete
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1)) {
  }

  // Enable interrupts for the end of each conversion
  stm32f10x_interrupt_nvic_enable(ADC1_2_IRQn, INTERRUPT_PRIORITY_HIGH);
  ADC_ITConfig(ADC1, ADC_IT_EOC, true);

  // If we are in continuous mode, start the adc
  if (adc_mode == ADC_MODE_CONTINUOUS) {
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  }

  // By default, enable vref and temp sensor for voltage conversions
  ADC_TempSensorVrefintCmd(true);
  adc_add_channel(ADC_REF);

  // Initialize static variables
  s_adc_status.continuous = adc_mode;
  s_adc_status.sequence = 0;
  s_adc_status.converting = xSemaphoreCreateBinaryStatic(&s_adc_status.converting_buf);
  s_adc_status.initialized = true;

  // If we are in continuous mode, start converting
  if (adc_mode == ADC_MODE_CONTINUOUS) {
    ADC_SoftwareStartConvCmd(ADC1, true);
  }
  return STATUS_CODE_OK;
}

void ADC1_COMP_IRQHandler() {
  // In case tasks are unblocked by behaviour in this IT handler
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // Interrupt EOC triggered after every conversion
  // Values generated in order of sequence, so we store them at channel pointed to by sequence index
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
    s_adc_sequence[s_adc_status.sequence]->reading = ADC_GetConversionValue(ADC1);
    if (s_adc_sequence[s_adc_status.sequence]->task != NULL) {
      // cannot use notify here because we need additional processing after notify
      // YIELD_FROM_ISR should be at the end of the function
      // even though it has same behaviour on stm32 as using notify_from_isr
      xTaskNotifyFromISR(s_adc_sequence[s_adc_status.sequence]->task->handle,
                         1u << s_adc_sequence[s_adc_status.sequence]->event, eSetBits,
                         &xHigherPriorityTaskWoken);
    }
    // If we've converted all the registered channels, start over
    if (++s_adc_status.sequence >= s_adc_status.active_channels) {
      s_adc_status.sequence = 0;
      // If single-shot, signal that all values converted
      if (s_adc_status.continuous == ADC_MODE_SINGLE) {
        xSemaphoreGiveFromISR(s_adc_status.converting, &xHigherPriorityTaskWoken);
        ADC_SoftwareStartConvCmd(ADC1, DISABLE);
      }
    }
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

StatusCode adc_read_raw(GpioAddress address, uint16_t *reading) {
  uint8_t channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  status_ok_or_return(prv_check_channel_enabled(channel));

  if (!s_adc_status.continuous) {
    // For Single-shot, we take semaphore and initiate a conversion
    xSemaphoreTake(s_adc_status.converting, portMAX_DELAY);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // Once conversion is finished, we will receive the semaphore from ISR
    xSemaphoreTake(s_adc_status.converting, portMAX_DELAY);
    xSemaphoreGive(s_adc_status.converting);
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
