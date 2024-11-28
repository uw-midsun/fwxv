#include <stdio.h>

#include "stm32f10x_rcc.h"
#include "stm32f10x_bkp.h"
#include "delay.h"
#include "log.h"
#include "tasks.h"
#include "stm32f10x.h"
#include "bkp.h"
#include "log.h"
#define BKP_OFFSET        (BKP_BASE - PERIPH_BASE)
#define CR_OFFSET         (BKP_OFFSET + 0x30)
#define TPE_BitNumber     0x00
#define TPAL_BitNumber    0x01
#define CR_TPE_BB         (PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPE_BitNumber * 4))
#define CR_TPAL_BB        (PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPAL_BitNumber * 4))
#define BKP_TamperPinLevel_High           ((uint16_t)0x0000)
#define BKP_TamperPinLevel_Low            ((uint16_t)0x0001)

//how do we initalize task?
TASK(bkp_task, TASK_STACK_512) { //512 bytes almost always enough
  LOG_DEBUG("Running\n");
  //testing write and read 
  BKP_WriteBackupRegister(32, 100); 
  LOG_DEBUG("DATA = %u\n", BKP_ReadBackupRegister(32)); 
  // LOG_DEBUG("i = %u\n", bkp_read(1)); 
  
  for (uint16_t i = 1; i <= 42; i++) {
        // bkp_write(i, i);  // Write each register with its index as test data (not permanent)
        uint16_t data = bkp_read(i); 
        LOG_DEBUG("data = %u\n", data); 
        delay_ms(10); 
        //assert if whatever is read is equal to whatever was just written
        if (data!=i) {
            LOG_DEBUG("Write/Read does not work\n, i = %u\n", i);
        }

        delay_ms(10); 
        LOG_DEBUG("i = %u\n", i);  

  }
  LOG_DEBUG("Test1\n");
  //testing clear 
  bkp_clear();
  for (int i = 1; i <= 42; ++i) {
          // Write each register with its index as test data (not permanent)
        // if (i==14) {
        //   LOG_DEBUG("2 is working"); 
        // }
        uint16_t data = bkp_read(i); 
        if (data!=0) {
          LOG_DEBUG("Clear does not work\n"); 
        }
        else {
          LOG_DEBUG("Clear working\n");
        }
        delay_ms(10); 
  }

  // StatusCode state = bkp_config_tamper(BKP_TamperPinLevel_Low, DISABLE);
  // if (state == (STATUS_CODE_OK && CR_TPE_BB == DISABLE) || (CR_TPAL_BB == BKP_TamperPinLevel_Low)) {
  //   LOG_DEBUG("test 1: Config Tamper works"); 
  // }
  // else {
  //   LOG_DEBUG("Config Tamper does not work");
  // }
  StatusCode state = bkp_config_tamper(BKP_TamperPinLevel_Low, DISABLE);

  if (state == STATUS_CODE_OK && *(__IO uint32_t*)CR_TPE_BB == DISABLE && *(__IO uint32_t *)CR_TPAL_BB == BKP_TamperPinLevel_Low) {
    LOG_DEBUG("Config Tamper works"); 
  }
  else {
    LOG_DEBUG("Config Tamper does not work");
  }

  delay_ms(10); 
  while (true) {

  }
}


int main() {
  tasks_init();
  log_init();
  bkp_init(); 
  tasks_init_task(bkp_task, TASK_PRIORITY(1), NULL); 
  tasks_start();
  return 0;
}