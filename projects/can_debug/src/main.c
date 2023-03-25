#include "can_debug.h"
#include "log.h"
#include "tasks.h"

#define FIRST_BYTE_HEX_MASK 0xFF
#define LAST_BYTE_HEX_MASK 0xFF00000000000000
#define LAST_BYTE_BIT_SHIFT 56

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
  .mode = 1,
};

void extract_first_byte(uint64_t data) {
  uint8_t first_byte = data & FIRST_BYTE_HEX_MASK;
  LOG_DEBUG("The first byte is: %X\n", first_byte);
}

void extract_last_byte(uint64_t data) {
  uint8_t last_byte = (data & LAST_BYTE_HEX_MASK) >> LAST_BYTE_BIT_SHIFT;
  LOG_DEBUG("The last byte is: %X\n", last_byte);
}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  while (true) {
    check_can_messages();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to CAN Debug!\n");
  can_init(&s_can_storage, &can_settings);

  CanMessageId first_byte_id = 0x01E;
  CanMessageId last_byte_id = 0x02E;

  can_debug_register(first_byte_id, extract_first_byte);
  can_debug_register(last_byte_id, extract_last_byte);

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
