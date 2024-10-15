#pragma once

enum prescaler_value {
  IWDG_Prescaler_4 = 4,
  IWDG_Prescaler_8 = 8,
  IWDG_Prescaler_16 = 16,
  IWDG_Prescaler_32 = 32,
  IWDG_Prescaler_64 = 64,
  IWDG_Prescaler_128 = 128,
  IWDG_Prescaler_256 = 256
};

void IWDG_Start(void);
void IWDG_Reload(void);
void IWDG_CalculateValues(uint16_t reset_time);
