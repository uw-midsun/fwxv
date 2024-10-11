#include "i_watchdog.h"

// taken from the original driver, this just defines prescaler values.
#define IWDG_Prescaler_4 ((uint8_t)0x00)
#define IWDG_Prescaler_8 ((uint8_t)0x01)
#define IWDG_Prescaler_16 ((uint8_t)0x02)
#define IWDG_Prescaler_32 ((uint8_t)0x03)
#define IWDG_Prescaler_64 ((uint8_t)0x04)
#define IWDG_Prescaler_128 ((uint8_t)0x05)
#define IWDG_Prescaler_256 ((uint8_t)0x06)

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
