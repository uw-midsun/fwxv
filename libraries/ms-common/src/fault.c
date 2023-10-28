#include "fault.h"

#include "delay.h"
#include "log.h"
#include "semphr.h"
#include "status.h"
#include "tasks.h"

#define FAULT_CYCLES 5

static FaultStorage *s_storage;

static uint8_t s_max_cycles_missed;
static uint32_t s_cycles_missed_bitset;

TASK(fault, TASK_MIN_STACK_SIZE) {
  bool fault = false;
  s_max_cycles_missed = 0;

  while (true) {
    if (!fault) {
      // Check faults
      for (uint8_t i = 0; i < s_storage->num_faults; i++) {
        if (s_storage->faults[i]) {
          fault = true;
        }
      }

      // Check all messages for timeout
      for (uint8_t i = 0; i < s_storage->num_faults; i++) {
        // If a message has not been received
        if (!s_storage->watching[i]) {
          s_storage->missed_cycles[i]++;
          if (s_storage->missed_cycles[i] > FAULT_CYCLES) {
            fault = true;
          }
        } else {
          s_storage->missed_cycles[i] = 0;
        }
      }

      // Delay until next check cycle
      delay_ms(100);
    } else {
      s_storage->handler();
    }
  }
}

StatusCode fault_init(FaultStorage *storage) {
  s_storage = storage;
  status_ok_or_return(tasks_init_task(fault, TASK_PRIORITY(4), NULL));
  return STATUS_CODE_OK;
}
