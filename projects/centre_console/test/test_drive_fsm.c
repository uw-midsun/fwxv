#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "drive_fsm_sequence.h"
#include "task_test_helpers.h"
#include "unity.h"

// buttons defined in drive_fsm.c

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

TEST_IN_TASK
void test_1(void) {
    init_drive_fsm();

    // neutral state
    fsm_run_cycle(drive);
    wait_tasks(1);
    TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);


    // 
    // gpio_set_state(&s_btn_start, GPIO_STATE_HIGH);
}