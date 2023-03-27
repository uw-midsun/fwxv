#include <stdio.h>
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "log.h"
#include "tasks.h"

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


TASK(smoke_adc_task, TASK_STACK_1024) {
   //for (uint8_t i = 0; i < 1; i++) {
   //   gpio_init_pin(&adc_addy[i], GPIO_ANALOG, GPIO_STATE_LOW);
   //}
   //adc_add_channel(tmp);
   //adc_add_channel(adc_addy[0]);
   //adc_init(ADC_MODE_SINGLE);
  GPIO_InitTypeDef GPIO_InitStructure = { 0 };
  
  RCC_ADCCLKConfig(RCC_PCLK2_Div2); 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  ADC_InitTypeDef ADC_InitStructure = { 0 };

  // By default, enable vref and temp sensor for voltage conversions
  ADC_TempSensorVrefintCmd(ENABLE);
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  ADC_TempSensorVrefintCmd(ENABLE);
  //ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
  ADC_TempSensorVrefintCmd(ENABLE);

  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

   while(true) {
    /* Wait for the conversion to complete */
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    
    /* Read the conversion result */
    uint16_t data = ADC_GetConversionValue(ADC1);

    LOG_DEBUG("voltage ref: %d\n\r", data);
    delay_ms(1000);
   }
}

int main() {
   tasks_init();
   interrupt_init();
   gpio_init();
   log_init();
   LOG_DEBUG("Welcome to ADC SMOKE TEST!\n\r");

   tasks_init_task(smoke_adc_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   return 0;
}
