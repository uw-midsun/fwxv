#include "power_select.h"
#include "task_test_helpers.h"
#include "unity.h"

static const GpioAddress g_power_supply_valid_pin = POWER_SELECT_PWR_SUP_VALID_ADDR;
static const GpioAddress g_dcdc_valid_pin = POWER_SELECT_DCDC_VALID_ADDR;
static const GpioAddress g_aux_bat_valid_pin = POWER_SELECT_AUX_BAT_VALID_ADDR;

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

TEST_IN_TASK
void test_power_select_power_supply_task(void) {
  init_power_supply();
  gpio_init_pin(&g_power_supply_valid_pin, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  fsm_run_cycle(power_supply);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_supply_fsm->curr_state->id, POWER_SELECT_INACTIVE);
  TEST_ASSERT_EQUAL(g_tx_struct.power_select_status_status, 0);

  gpio_set_state(&g_power_supply_valid_pin, GPIO_STATE_LOW);
  fsm_run_cycle(power_supply);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_supply_fsm->curr_state->id, POWER_SELECT_ACTIVE);
  TEST_ASSERT_EQUAL(g_tx_struct.power_select_status_status, POWER_SELECT_PWR_SUP_STATUS_MASK);

  fsm_run_cycle(power_supply);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_supply_fsm->curr_state->id, POWER_SELECT_ACTIVE);
  TEST_ASSERT_EQUAL(g_tx_struct.power_select_status_status, POWER_SELECT_PWR_SUP_STATUS_MASK);

  gpio_set_state(&g_power_supply_valid_pin, GPIO_STATE_HIGH);
  fsm_run_cycle(power_supply);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_supply_fsm->curr_state->id, POWER_SELECT_INACTIVE);
  TEST_ASSERT_EQUAL(g_tx_struct.power_select_status_status, 0);

  fsm_run_cycle(power_supply);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_supply_fsm->curr_state->id, POWER_SELECT_INACTIVE);
  TEST_ASSERT_EQUAL(g_tx_struct.power_select_status_status, 0);
}
