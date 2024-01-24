#include "adc.h"

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "gpio.h"
#include "log.h"
#include "semaphore.h"
#include "status.h"
#include "stm32f10x.h"
#include "stm32f10x_interrupt.h"

// Sample rate for all ADC channels
#define ADC_SAMPLE_RATE ADC_SampleTime_239Cycles5
#define ADC_MAX_VAL 4095
#define MAX_ADC_READINGS 16

// Temp sensor avg voltage @25 celcius and avg temp slope (mV/C)
// Defined in section 5 of the datasheet
#define V25_CALIB_VAL 1430
#define TEMP_AVG_SLOPE 4

// Vref is used as a 1200mV reference signal
// For more accurate voltage readings
#define VREFINT_MV 1200

typedef struct AdcStatus {
  bool initialized;
  uint8_t active_channels;  // Keeps track of how many channels have been registered with the ADC
  Semaphore converting;
} AdcStatus;

static AdcStatus s_adc_status;

// Store of ranks for each channel, at their channel index
static uint8_t s_adc_ranks[NUM_ADC_CHANNELS];

// Array of readings, in order of rank, which DMA will write to
static volatile uint16_t s_adc_readings[MAX_ADC_READINGS];

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

// Formula obtained from section 11.10 of the reference manual
// Returns the temperature in celsius*100
static uint16_t prv_get_temp(uint16_t reading, uint16_t vref) {
  if (V25_CALIB_VAL - reading > 0) {
    return (V25_CALIB_VAL - reading) * 100 / TEMP_AVG_SLOPE + 25;
  }
  return 0;
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

  // Enable ADC1/DMA1 Clock
  RCC_ADCCLKConfig(RCC_PCLK2_Div2);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  // DMA initialization needed for multi-channel scan
  DMA_DeInit(DMA1_Channel1);
  DMA_InitTypeDef dma_init = {
    .DMA_PeripheralBaseAddr = (uint32_t) & (ADC1->DR),
    .DMA_MemoryBaseAddr = (uint32_t)s_adc_readings,
    .DMA_DIR = DMA_DIR_PeripheralSRC,
    .DMA_BufferSize = s_adc_status.active_channels,
    .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
    .DMA_MemoryInc = DMA_MemoryInc_Enable,
    .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
    .DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
    .DMA_Mode = DMA_Mode_Circular,
    .DMA_Priority = DMA_Priority_High,
    .DMA_M2M = DMA_M2M_Disable,
  };
  DMA_Init(DMA1_Channel1, &dma_init);

  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA1_Channel1, ENABLE);

  // Setup ADC1 for use
  // Clear Previous settings
  ADC_DeInit(ADC1);
  ADC_InitTypeDef ADC_InitStructure = { 0 };

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = s_adc_status.active_channels;
  ADC_Init(ADC1, &ADC_InitStructure);

  // Initialize all channels which have a designated rank
  for (uint8_t channel = 0; channel < NUM_ADC_CHANNELS; channel++) {
    // Each channel is configured with ascending rank, starting at one
    if (s_adc_ranks[channel] != 0) {
      ADC_RegularChannelConfig(ADC1, channel, s_adc_ranks[channel], ADC_SAMPLE_RATE);
    }
  }

  // By default, enable vref and temp sensor for voltage conversions
  ADC_TempSensorVrefintCmd(true);

  // Enable the ADC
  ADC_Cmd(ADC1, true);
  ADC_TempSensorVrefintCmd(ENABLE);
  ADC_DMACmd(ADC1, ENABLE);

  // Calibrate ADC
  // Reset Calibration regs, wait for flag to be cleared
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1)) {
  }
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1)) {
  }

  // Initialize static variables
  sem_init(&s_adc_status.converting, 1, 0);
  DMA_ClearITPendingBit(DMA1_IT_TC1);
  DMA_ClearITPendingBit(DMA1_IT_HT1);
  DMA_ClearITPendingBit(DMA1_IT_GL1);

  // Enable interrupts for the end of each full conversion
  stm32f10x_interrupt_nvic_enable(DMA1_Channel1_IRQn, INTERRUPT_PRIORITY_LOW);

  // goes into ISR after enabling interrupts so sem_wait here to reset semaphore
  status_ok_or_return(sem_wait(&s_adc_status.converting, ADC_TIMEOUT_MS));

  s_adc_status.initialized = true;

  return STATUS_CODE_OK;
}

StatusCode adc_run(void) {
  // Initiate a conversion of all channels, and wait for semaphore from ISR
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
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
    *reading = prv_get_temp(adc_reading, vref);
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

// Don't need to do anything on ARM
void adc_deinit(void) {}

void DMA1_Channel1_IRQHandler() {
  BaseType_t higher_prio;
  xSemaphoreGiveFromISR(s_adc_status.converting.handle, &higher_prio);
  DMA_ClearITPendingBit(DMA1_IT_TC1);
  DMA_ClearITPendingBit(DMA1_IT_HT1);
  DMA_ClearITPendingBit(DMA1_IT_GL1);
  portYIELD_FROM_ISR(higher_prio);
}
