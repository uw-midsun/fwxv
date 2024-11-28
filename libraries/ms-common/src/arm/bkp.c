#include "flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bkp.h"
#include "log.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_bkp.h"

void bkp_init() {
/*
    enable power and backup interface clocks by setting PWREN and BKPEN bits in
    RCC_APB1ENR register
*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
}




uint16_t bkp_read(uint16_t reg) {
    if(reg>=1 && reg<=42){
        if (reg > 10) {
            return BKP_ReadBackupRegister(reg*4+0x0014);
        }
        else {
            return BKP_ReadBackupRegister(reg*4);
        }
    }
    else {
        return 0;
    }
}

StatusCode bkp_write(uint16_t reg, uint16_t data){
    if (reg >= 1 && reg <= 42) {
        if (reg > 10) {
            BKP_WriteBackupRegister(reg*4+0x0014, data); 
        }
        else {
            BKP_WriteBackupRegister(reg*4, data);
        }
        return STATUS_CODE_OK;
    }
    else{
        return STATUS_CODE_INVALID_ARGS;
    }
    
}

StatusCode bkp_clear(){
    for (int i=1; i<=42; i++) { 
        bkp_write(i*4, 0); 
    } 
    return STATUS_CODE_OK; 
   
}

StatusCode bkp_config_tamper(uint16_t TamperPinLevel, FunctionalState State){
    if(TamperPinLevel == BKP_TamperPinLevel_High){
        BKP_TamperPinLevelConfig(BKP_TamperPinLevel_High);
        BKP_TamperPinCmd(State);
        return STATUS_CODE_OK;
    }
    else if(TamperPinLevel == BKP_TamperPinLevel_Low) {
        BKP_TamperPinLevelConfig(BKP_TamperPinLevel_Low);
        BKP_TamperPinCmd(State);
        return STATUS_CODE_OK;   
    }
    else{
        return STATUS_CODE_INVALID_ARGS;
    }
}