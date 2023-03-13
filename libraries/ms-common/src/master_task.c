#include "master_task.h"

static uint32_t MASTER_MS_CYCLE_TIME = 100;

#define MASTER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define MAX_CYCLES_OVER 5

static uint8_t s_cycles_over = 1;

static uint8_t s_medium_cycle_count = 10;
static uint8_t s_slow_cycle_count = 100;

void set_master_cycle_time(uint8_t time_ms) {
  MASTER_MS_CYCLE_TIME = time_ms;
}
void set_medium_cycle_count(uint8_t cycles) {
  s_medium_cycle_count = cycles;
}
void set_slow_cycle_count(uint8_t cycles) {
  s_slow_cycle_count = cycles;
}

void run_fast_cycle();
void run_medium_cycle();
void run_slow_cycle();

void check_late_cycle(BaseType_t delay) {
    if (delay != pdTRUE) {
        LOG_WARN("Master Task out of sync!! Currently over by %u cycles\n", s_cycles_over);
        ++s_cycles_over;
    }
    if (s_cycles_over > MAX_CYCLES_OVER) {
        vTaskEndScheduler();
    }
}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t counter = 0;
  while (true) {
#ifdef TEST
    xSemaphoreTake(test_cycle_start_sem);
#endif
    run_fast_cycle();
    if (!(counter % s_medium_cycle_count)) run_medium_cycle();
    if (!(counter % s_slow_cycle_count)) run_slow_cycle();

#ifdef TEST
    xSemaphoreGive(test_cycle_end_sem);
#endif
    // TODO: perhaps also use xTaskCheckForTimeOut()?
    BaseType_t delay = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MASTER_MS_CYCLE_TIME));
    check_late_cycle(delay);
    ++counter;
  }
}

StatusCode init_master_task() {
  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}

Task* get_master_task() {
  return master_task;
}
