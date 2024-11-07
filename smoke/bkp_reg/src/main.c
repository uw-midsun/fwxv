#include <stdio.h>

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




//how do we initalize task?
TASK(bkp_task, TASK_STACK_512) { //512 bytes almost always enough

  //testing write and read 
  for (int i = 1; i <= 42; i++) {
        bkp_write(i, i);  // Write each register with its index as test data (not permanent)
        uint16_t data = bkp_read(32*i); 
        //assert if whatever is read is equal to whatever was just written
        if (data!=i) {
            LOG_DEBUG("Write/Read does not work");
        }
  }

  //testing clear 
  bkp_clear();
  for (int i = 1; i <= 42; i++) {
          // Write each register with its index as test data (not permanent)
        uint16_t data = bkp_read(32*i); 
        if (data!=0) {
          LOG_DEBUG("Clear does not work"); 
        }
  }

  //testing config tamper 

  // should we consider 
  StatusCode state = bkp_config_tamper(1, DISABLE); 
  // ask aryan 
  if (state == STATUS_CODE_OK && CR_TPE_BB == 0 && CR_TPAL_BB == 1) {
  
    LOG_DEBUG("Config Tamper works"); 
  }
  else {
    LOG_DEBUG("Config Tamper does not work");
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