#include "can.h"
#include "log.h"
#include "tasks.h"
#include "can_board_ids.h"

#define TARGET_HEX_MASK 0xF0
#define TARGET_BIT_SHIFT 4
#define SIZE_OF_UINT32 4

// Add telemetry CAN message IDs here
static const CanMessageId telemetry_ids[] = {
  SYSTEM_CAN_MESSAGE_TELEMETRY_TEST_MSG,
};

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
  .mode = CAN_ONE_SHOT_MODE,
};

// Function to pop CAN messages off the queue if they are addressed to telemetry
StatusCode telemetry_queue_pop(CanMessage *msg) {
  if (can_receive(msg) == STATUS_CODE_OK) {
    for (uint8_t i = 0; i < sizeof(telemetry_ids) / SIZE_OF_UINT32; i++) {
      if (telemetry_ids[i] == msg->id.raw) {
        return STATUS_CODE_OK;
      }
    }
  }
  memset(msg, 0, sizeof(*msg));
  return STATUS_CODE_EMPTY;
}

// Sample task using telemetry_queue_pop to receive telemetry messages
TASK(master_task, TASK_MIN_STACK_SIZE) {
  CanMessage msg = { 0 };
  while (true) {
    if (telemetry_queue_pop(&msg) == STATUS_CODE_OK) {
      LOG_DEBUG("Received a message!\n");
    }
  }
}

int main() {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to Telemetry!\n");
  can_init(&s_can_storage, &can_settings);

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
