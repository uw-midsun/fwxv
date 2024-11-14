
#include "wwdg.h"
#include "stm32f10x_wwdg.h"
#include "gpio.h"
#include "log.h"
#include "semaphore.h"
#include "status.h"
#include "task_test_helpers.h"
#include "wwdg.h"



TASK(wwdg_test_task, TASK_STACK_512) {
uint32_t prescaler=WWDG_Prescaler_1;
uint8_t window_value=0x1F;
uint8_t counter_value=0x7f;




while (true) {
    WWDG_Refresh(counter_value);
    LOG_DEBUG("successful refresh %u\n", counter_value);
        delay_ms(500);
    }
    
}

int main(void) {
    gpio_init();
    log_init();
    tasks_init_task(wwdg_test_task, TASK_PRIORITY(2), NULL);
    tasks_start();
    LOG_DEBUG("WWDG test task started...\n");
    while (1) {}
    return 0;
}
//poo
