#include "can_debug.h"
#include "log.h"

static int s_num_can_callback_functions;
static CanDebugCallback callback_functions[MAX_NUM_CALLBACKS];
static CanMessageId can_ids[MAX_NUM_CALLBACKS];

StatusCode can_debug_init() {
  s_num_can_callback_functions = 0;
  return STATUS_CODE_OK;
}

StatusCode can_debug_register(CanMessageId id, CanDebugCallback callback) {
  if (s_num_can_callback_functions < MAX_NUM_CALLBACKS) {
    callback_functions[s_num_can_callback_functions] = callback;
    can_ids[s_num_can_callback_functions] = id;
    s_num_can_callback_functions++;
    return STATUS_CODE_OK;
  } else {
    LOG_CRITICAL("The max number of callbacks has been reached!");
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
}

void check_can_messages() {
  CanMessage msg = { 0 };
  while (can_receive(&msg) == STATUS_CODE_OK) {
    for (int i = 0; i < MAX_NUM_CALLBACKS; i++) {
      if (can_ids[i] == msg.id.raw) {
        (*callback_functions[i])(msg.data);
      }
    }
  }
}
