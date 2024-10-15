#include "i_watchdog.h"

#include "stm32f10x_iwdg.h"

void IWDG_SetValues(uint16_t prescaler_value) {
  // Unlock register to write to
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  // Set the prescaler value using the #define from above and write it into the IWDG_PR register
  IWDG_SetPrescaler(PRESCALER_VALUE);

  // Set the reload value using #define from above and write it into IWDG_WLR register
  IWDG_SetReload(RELOAD_VALUE);

  // Reload the IWDG with the value set previously in IWDG_SetReload
  IWDG_ReloadCounter();
}

void IWDG_Reload(void) {
  IWDG_ReloadCounter();
}
