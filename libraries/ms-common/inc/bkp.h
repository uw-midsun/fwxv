#include <stdio.h>
#include "status.h"
#include "stm32f10x.h"

void bkp_init();
uint16_t bkp_read(uint16_t reg);
StatusCode bkp_write(uint16_t reg, uint16_t data);
StatusCode bkp_clear();
StatusCode bkp_config_tamper(uint16_t TamperPinLevel, FunctionalState State);
