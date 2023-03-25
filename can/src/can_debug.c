#include "can_debug.h"
#include "log.h"

static int i;
static CanMessage msg;
static CanDebugCallback callback_functions[NUM_CALLBACKS];
static CanMessageId can_ids[NUM_CALLBACKS];

StatusCode can_debug_init() {
  i = 0;
  memset(&msg, 0, sizeof(msg));
  return STATUS_CODE_OK;
}

StatusCode can_debug_register(CanMessageId id, CanDebugCallback callback) {
  if (i < NUM_CALLBACKS) {
    callback_functions[i] = callback;
    can_ids[i] = id;
    i++;
    return STATUS_CODE_OK;
  } else {
    LOG_CRITICAL("The max number of callbacks has been reached!");
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
}

void check_can_messages() {
  if (can_receive(&msg) == STATUS_CODE_OK) {
    for (int i = 0; i < NUM_CALLBACKS; i++) {
      if (can_ids[i] == msg.id.raw) {
        (*callback_functions[i])(msg.data);
      }
    }
  }
}
