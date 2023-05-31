#include <stdio.h>
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "log.h"
#include "tasks.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_interrupt.h"
#include "stm32f10x_adc.h"

#define ADC_TIMEOUT_MS 100

static const GpioAddress adc_addy[] = {
  { .port = GPIO_PORT_A, .pin = 0 },   
  { .port = GPIO_PORT_A, .pin = 1 },   
  { .port = GPIO_PORT_A, .pin = 2 },
  { .port = GPIO_PORT_A, .pin = 4 },
  { .port = GPIO_PORT_A, .pin = 5 },
  { .port = GPIO_PORT_A, .pin = 6 },
  { .port = GPIO_PORT_A, .pin = 7 },
  { .port = GPIO_PORT_B, .pin = 0 },
  { .port = GPIO_PORT_B, .pin = 1 },
};

static const GpioAddress tmp = 
  { .port = NUM_GPIO_PORTS, .pin = ADC_Channel_Vrefint };


static Semaphore converting;

volatile uint16_t adc_values[16];

TASK(smoke_adc_task, TASK_STACK_1024) {

  // GPIO/RCC Initialization
  GPIO_InitTypeDef GPIO_InitStructure = { 0 };
  RCC_ADCCLKConfig(RCC_PCLK2_Div2); 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  for (int i = 0; i < 9; i++) {
    gpio_init_pin(&adc_addy[i], GPIO_ANALOG, GPIO_STATE_LOW);
  }
  
  // DMA initialization needed for multi-channel scan
  DMA_DeInit(DMA1_Channel1);
  DMA_InitTypeDef dma_init = {
    .DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR),
    .DMA_MemoryBaseAddr = (uint32_t)adc_values,
    .DMA_DIR = DMA_DIR_PeripheralSRC,
    .DMA_BufferSize = 16,
    .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
    .DMA_MemoryInc = DMA_MemoryInc_Enable,
    .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
    .DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
    .DMA_Mode = DMA_Mode_Normal,
    .DMA_Priority = DMA_Priority_High,
    .DMA_M2M = DMA_M2M_Disable,
  };
  DMA_Init(DMA1_Channel1, &dma_init);

  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA1_Channel1, ENABLE);

  ADC_InitTypeDef ADC_InitStructure = { 0 };

  // By default, enable vref and temp sensor for voltage conversions
  ADC_TempSensorVrefintCmd(ENABLE);
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 16;
  ADC_Init(ADC1, &ADC_InitStructure);

  ADC_TempSensorVrefintCmd(ENABLE);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 9, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 10, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 11, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 12, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 13, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 14, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 15, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 16, ADC_SampleTime_55Cycles5);
  ADC_TempSensorVrefintCmd(ENABLE);


  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  stm32f10x_interrupt_nvic_enable(DMA1_Channel1_IRQn, INTERRUPT_PRIORITY_LOW);
  sem_init(&converting, 1, 1); 

   while(true) {
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    /* Wait for the conversion to complete */
    sem_wait(&converting, ADC_TIMEOUT_MS);
    for (int i = 0; i < 16; i++) {
      printf("ADC %d: %d\n\r", i, adc_values[i]);
    }
    
    /* Read the conversion result */
    delay_ms(1000);
   }
}

int main() {
   tasks_init();
   interrupt_init();
   gpio_init();
   log_init();

   tasks_init_task(smoke_adc_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   return 0;
}


void DMA1_Channel1_IRQHandler() { 
  BaseType_t xHigherPriorityTaskWoken;
  xSemaphoreGiveFromISR(converting.handle, &xHigherPriorityTaskWoken);
  DMA_ClearITPendingBit(DMA1_IT_TC1);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

