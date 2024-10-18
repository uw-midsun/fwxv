#pragma once

// An enum for the supported prescaler values
typedef enum {
  IWDG_Prescaler_4 = 0x00,
  IWDG_Prescaler_8,
  IWDG_Prescaler_16,
  IWDG_Prescaler_32,
  IWDG_Prescaler_64,
  IWDG_Prescaler_128,
  IWDG_Prescaler_256
} IWDGPrescalerValue;

void IWDG_Start(void);
void IWDG_Reload(void);
void IWDG_CalculateValues(uint16_t reset_time, uint16_t reload_period_ms);
