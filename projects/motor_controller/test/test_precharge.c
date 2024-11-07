#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_codegen.h"
#include "delay.h"
#include "fsm.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "misc.h"
#include "motor_can.h"
#include "motor_controller_setters.h"
#include "precharge.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

#define PRECHARGE_EVENT 0

static PrechargeSettings precharge_settings = {
  .motor_sw = { GPIO_PORT_A, 9 },
  .precharge_monitor = { GPIO_PORT_B, 0 },
};

bool initialized = false;

void setup_test(void) {
  if (initialized) return;
  initialized = true;

  interrupt_init();
  gpio_it_init();
  tasks_init();
  log_init();

  GpioState state;
  gpio_get_state(&precharge_settings.precharge_monitor, &state);
  if (state == GPIO_STATE_LOW) {
    gpio_toggle_state(&precharge_settings.precharge_monitor);
  }

  gpio_get_state(&precharge_settings.precharge_monitor, &state);
  precharge_init(&precharge_settings, PRECHARGE_EVENT, test_task);
}

void teardown_test(void) {}

// from main.c
void run_fast_cycle() {
  uint32_t notification;
  notify_get(&notification);
  if (notification & (1 << PRECHARGE_EVENT)) {
    LOG_DEBUG("Precharge complete\n");
    set_mc_status_precharge_status(true);
  }
}

TEST_IN_TASK
void test_precharge(void) {
  // expect to have tx struct mc_status_precharge_status to be false on startup
  TEST_ASSERT_FALSE(g_tx_struct.mc_status_precharge_status);

  gpio_it_trigger_interrupt(&precharge_settings.precharge_monitor);

  // run cycle once
  run_fast_cycle();

  // expect to have tx struct mc_status_precharge_status set to true
  TEST_ASSERT_TRUE(g_tx_struct.mc_status_precharge_status);
}
