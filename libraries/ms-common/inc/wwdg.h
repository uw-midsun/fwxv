#pragma once

#include "status.h"

typedef enum {
  WWDG_Prescaler_1 = 0x00000000,
  WWDG_Prescaler_2 = 0x00000080,
  WWDG_Prescaler_4 = 0x00000100,
  WWDG_Prescaler_8 = 0x00000180
} WWDGPrescalerValue;

StatusCode WWDG_Start(void);
StatusCode WWDG_Reload(void);
StatusCode WWDG_CalculateValues(uint16_t reset_time, uint16_t reload_period_ms);