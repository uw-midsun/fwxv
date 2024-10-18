#include "i_watchdog.h"

#include "stm32f10x_iwdg.h"

void IWDG_SetValues(IWDGPrescalerValue prescaler_value, uint16_t reload_period_ms) {
  // Unlock register to write to
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  // Set the prescaler value using the #define from above and write it into the IWDG_PR register
  IWDG_SetPrescaler((uint8_t)prescaler_value);

  // Set the reload value using #define from above and write it into IWDG_WLR register
  // ((reload_period_ms * 40000) / ((4 * (1 << prescaler_value)) * 1000)) <- The calculation used
  // but then simplified
  IWDG_SetReload((reload_period_ms / (1 << prescaler_value)) * 10);

  // Reload the IWDG with the value set previously in IWDG_SetReload
  IWDG_ReloadCounter();

  // Start the iwdg
  IWDG_Enable();
}

// A function to reload the counter (i.e. reset its time)
void IWDG_Reload(void) {
  IWDG_ReloadCounter();
}
