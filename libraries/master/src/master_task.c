#include "master_task.h"

static uint32_t MASTER_MS_CYCLE_TIME = 100;

#define MASTER_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define MAX_CYCLES_OVER 5

static uint8_t s_cycles_over = 0;

static uint32_t s_medium_cycle_count = 10;
static uint32_t s_slow_cycle_count = 100;

void set_master_cycle_time(uint32_t time_ms) {
  MASTER_MS_CYCLE_TIME = time_ms;
}
void set_medium_cycle_count(uint32_t cycles) {
  s_medium_cycle_count = cycles;
}
void set_slow_cycle_count(uint32_t cycles) {
  s_slow_cycle_count = cycles;
}

#ifdef MS_TEST
uint8_t get_cycles_over() {
  return s_cycles_over;
}
#endif

void run_fast_cycle();
void run_medium_cycle();
void run_slow_cycle();
void pre_loop_init();

void check_late_cycle(BaseType_t delay) {
  if (delay != pdTRUE) {
    ++s_cycles_over;
    LOG_WARN("Master Task out of sync!! Currently over by %u cycles\n", s_cycles_over);
  } else {
    if (s_cycles_over != 0) {
      --s_cycles_over;
    }
  }
  if (s_cycles_over > MAX_CYCLES_OVER) {
    LOG_CRITICAL("Master Task out of sync!! Ending Scheduler\n");
    vTaskEndScheduler();
  }
}

TASK(master_task, TASK_STACK_512) {
  pre_loop_init();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t counter = 1;
  while (true) {
    // LOG_DEBUG("counter: %u\n", counter);

    run_fast_cycle();
    if (!(counter % s_medium_cycle_count)) run_medium_cycle();
    if (!(counter % s_slow_cycle_count)) run_slow_cycle();

    // TODO: perhaps also use xTaskCheckForTimeOut()?
    BaseType_t delay = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MASTER_MS_CYCLE_TIME));
    check_late_cycle(delay);
    ++counter;
  }
}

StatusCode init_master_task() {
  s_cycles_over = 0;
  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}

Task *get_master_task() {
  return master_task;
}
