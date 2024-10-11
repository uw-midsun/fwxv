#include "i_watchdog.h"

void IWDG_Start(void) {
  IWDG_Enable();
  IWDG_SetPrescaler(32);
  IWDG_SetReload(3000);
}

void IWDG_Reload() {
  IWDG_ReloadCounter();
}

void IWDG_Stop(void) {
  // Need to implement
}
