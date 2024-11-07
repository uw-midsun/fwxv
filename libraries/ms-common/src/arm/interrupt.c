#include "interrupt.h"

#include "stm32f10x_interrupt.h"

void interrupt_init(void) {
  stm32f10x_interrupt_init();
}
