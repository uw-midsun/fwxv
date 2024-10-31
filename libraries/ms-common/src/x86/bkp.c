#include "flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

// Look into which pins cant be changed and can only be read

int const address_default = 0x00;
void bkp_clear() {
	// change to 0x34 which is BKP_CSR
    uint16_t register = address_default + 0x34;
    int bit1_change =(1 << 1);
    int bit0_change = 1;
    register |= bit1_change;
    register |= bit0_change;	
}


// Sets to pins to default values
void bkp_init() {
/*
    enable power and backup interface clocks by setting PWREN and BKPEN bits in
    RCC_APB1ENR register
*/

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




void bkp_read(uint8_t register) {
    // if(register >= 1 && register <= 42){
    //     BKP_DR1
    // }
    BKP
    
}

void bkp_write(uint8_t register, uint16_t data){
    if(register >= 1 && register <= 42){
        
    }
}

// void bkp_clear();

//void bkp_config_tamper();








