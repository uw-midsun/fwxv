#include "wwdg.h"

#include "stm32f10x_wwdg.h"



void WWDG_Setup(uint32_t prescaler, uint8_t window_value, uint8_t counter_value) {
    WWDG_DeInit();

    WWDG_SetPrescaler(prescaler);

    WWDG_SetWindowValue(window_value);

    WWDG_Enable(counter_value);
}

void WWDG_Refresh(uint8_t counter_value) {
    WWDG_SetCounter(counter_value);
}

void WWDG_IRQHandler(){
uint8_t counter_value=0xff;
FlagStatus check=WWDG_GetFlagStatus();
if (check) {
    WWDG_Refresh(counter_value);
    WWDG_ClearFlag();
}   
}

//poo