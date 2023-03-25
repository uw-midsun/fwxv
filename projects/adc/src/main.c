#include <stdio.h>

#include "log.h"
#include "adc.h"
#include "tasks.h"
#include "delay.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

ADC_InitTypeDef ADC_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;

void init_ADC(void)
{
    // Enable the ADC1 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // Configure ADC pins as analog inputs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure the ADC
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 8;
    ADC_Init(ADC1, &ADC_InitStructure);

    // Enable the ADC
    ADC_Cmd(ADC1, ENABLE);

    // Calibrate the ADC
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

uint16_t read_ADC(uint8_t channel)
{
    // Set the channel to read
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_55Cycles5);

    // Start the conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // Wait for the conversion to complete
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    // Read the result
    return ADC_GetConversionValue(ADC1);
}

TASK(master_task, TASK_STACK_512) {

 // Initialize the ADC
    init_ADC();

    // Read all ADC channels
    uint16_t readings[8];
  while (1)
  {
    for (int i = 0; i < 8; i++) {
        readings[i] = read_ADC(i);
    LOG_DEBUG("ADC Value: %d\n\r", readings[i]); 
    delay_ms(100);
    }

  }

}

int main() {
   tasks_init();
   log_init();
   LOG_DEBUG("Welcome to TEST!");

   tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   LOG_DEBUG("exiting main?");
   return 0;
}

