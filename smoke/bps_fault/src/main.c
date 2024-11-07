#include <stdio.h>
#include <string.h>

#include "flash.h"
#include "log.h"
#include "master_task.h"
#include "persist.h"
#include "tasks.h"

typedef struct BpsStorage {
  uint16_t fault_bitset;
  uint16_t vehicle_speed;
} BpsStorage;

#define BPS_FAULT_FLASH_PAGE NUM_FLASH_PAGES - 1
static BpsStorage s_bps_storage;
static PersistStorage s_persist;

TASK(clear_bps_fault, TASK_STACK_512) {
  memset(&s_bps_storage, 0, sizeof(s_bps_storage));
  persist_init(&s_persist, BPS_FAULT_FLASH_PAGE, &s_bps_storage, sizeof(s_bps_storage), true);
  persist_ctrl_periodic(&s_persist, false);
  LOG_DEBUG("FAULT_BITSET: %d\n", s_bps_storage.fault_bitset);
  LOG_DEBUG("VEHICLE SPEED: %d\n", s_bps_storage.vehicle_speed);
  s_bps_storage.fault_bitset = 0;
  s_bps_storage.vehicle_speed = 0;
  persist_commit(&s_persist);
  LOG_DEBUG("FAULT_BITSET CLEARED: %d\n", s_bps_storage.fault_bitset);

  while (true) {
  }
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(clear_bps_fault, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
