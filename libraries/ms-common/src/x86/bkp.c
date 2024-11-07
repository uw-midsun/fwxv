#include "flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_bkp.h"

// Look into which pins cant be changed and can only be read

// int const address_default = 0x00;
// void bkp_clear() {
// 	// change to 0x34 which is BKP_CSR
//     uint16_t register = address_default + 0x34;
//     int bit1_change =(1 << 1);
//     int bit0_change = 1;
//     register |= bit1_change;
//     register |= bit0_change;	
// }


// Sets to pins to default values
void bkp_init() {
/*
    enable power and backup interface clocks by setting PWREN and BKPEN bits in
    RCC_APB1ENR register
*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);


    
    int PWREN_ON = 1;
    PWREN_ON <<= 28;
    int BKPEN_ON = 1;
    BKPEN_ON <<= 27;
    //is this the right one?
    // uint32_t *RCC_APB1ENR = RCC_APB1ENR ;
    // *RCC_APB1ENR |= PWREN_ON;
    // *RCC_APB1ENR |= BKPEN_ON;
    // RCC is pointer, -> means to access the APB1ENR member of the structure pointed to by RCC
    RCC->APB1ENR |= PWREN_ON;
    RCC->APB1ENR |= BKPEN_ON;
    // set the DBP bit in the Power control register (PWR_CR) to enable access to the
    // Backup registers and RTC.
    int DBP_ON = 1;
    DBP_ON <<= 8;
    PWR->CR |= DBP_ON;
}

// Set everything to our desired bit values
// 6.4.3 BKP_CR
// Bit 0 = 1
// Bit 1 = 0 or 1 (not sure)
// 6.4.4 BKP_CSR
// Bit 0 = 0
// Bit 1 = 0
// Bit 2 = 0
// Bit 3 = 0
// Bit 8 = 0
// Bit 9 = 0


uint16_t bkp_read(uint16_t reg) {
    if(reg>=1 && reg <=42){
        BKP_ReadBackupRegister(reg);
    }
    // what should we return instead?

}

StatusCode bkp_write(uint16_t reg, uint16_t data){

    if (reg >= 1 && reg <= 42) {
        BKP_WriteBackupRegister(reg*32, data);
        return STATUS_CODE_OK;
    }
    else{
        return STATUS_CODE_INVALID_ARGS;
    }
    
}

StatusCode bkp_clear(){
    
    for (int i=1; i<=42; i++) { 
        bkp_write(i, 0); 
    } 
    return STATUS_CODE_OK; 
   
}

StatusCode bkp_config_tamper(uint16_t TamperPinLevel, FunctionalState State){
    if(TamperPinLevel == 1 || TamperPinLevel == 0){
        BKP_TamperPinLevelConfig(TamperPinLevel^1U);
        BKP_TamperPinCmd(State);
        return STATUS_CODE_OK;
    }
    else{
        return STATUS_CODE_INVALID_ARGS;
    }
}

StatusCode smoke_test(){
    /*
        create a .h file  
        create TASK that tests

    */ 


}







