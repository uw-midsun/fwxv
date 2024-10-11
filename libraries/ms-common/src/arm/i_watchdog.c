#include "i_watchdog.h"

#include "stm32f10x_iwdg.h"

// This is the time that we want the clock to reset after
#define RELOAD_TIME ((uint16_t)3000)

// This is a prescaler value that scales a 32 kHz clock (LSI)
#define PRESCALER_VALUE (IWDG_Prescaler_32)

// To get the reload value based on how the prescaler is defined
#define RELOAD_VALUE ((uint16_t)(REALOAD_TIME(32000 / PRESCALER_VALUE))

void IWDG_Start(void) {
  IWDG_Enable();
  IWDG_SetPrescaler(PRESCALER_VALUE);
  IWDG_SetReload(SET_RELOAD_VALUE);
}

void IWDG_Reload() {
  IWDG_ReloadCounter();
}
