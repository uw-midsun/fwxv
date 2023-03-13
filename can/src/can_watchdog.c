#include "can_watchdog.h"

#include "log.h"

static bool s_can_watchdogs_awake = false;

typedef struct CanWatchDog {
  CanMessageId msgId;
  uint16_t cycles_over;
  uint16_t max_cycles;
  bool checked;
} CanWatchDog;

static CanWatchDog s_can_watchdogs[CAN_QUEUE_SIZE];
static uint8_t s_num_can_watchdogs;

StatusCode can_watchdogs_init() {
  s_can_watchdogs_awake = true;
  s_num_can_watchdogs = 0;
  return STATUS_CODE_OK;
}

bool can_watchdogs_awake() {
  return s_can_watchdogs_awake;
}

StatusCode can_add_watchdog(CanMessageId msg_id, uint16_t max_cycles) {
  if (msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN Watchdog: Invalid message ID");
  } else if (!s_can_watchdogs_awake) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "CAN Watchdog: CAN Watchdogs not awake");
  }

  s_can_watchdogs[s_num_can_watchdogs].msgId = msg_id;
  s_can_watchdogs[s_num_can_watchdogs].cycles_over = 0;
  s_can_watchdogs[s_num_can_watchdogs].max_cycles = max_cycles;
  s_can_watchdogs[s_num_can_watchdogs].checked = false;

  ++s_num_can_watchdogs;

  return STATUS_CODE_OK;
}

// TODO: make this search faster or shove this into autogen
StatusCode watchdog_check(CanMessageId msg_id) {
  for (size_t i = 0; i < s_num_can_watchdogs; ++i) {
    if (s_can_watchdogs[i].msgId == msg_id) {
      s_can_watchdogs[i].checked = true;
      s_can_watchdogs[i].cycles_over = 0;
      break;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode check_can_watchdogs() {
  for (size_t i = 0; i < s_num_can_watchdogs; ++i) {
    if (!s_can_watchdogs[i].checked) {
      ++s_can_watchdogs[i].cycles_over;
      if (s_can_watchdogs[i].cycles_over >= s_can_watchdogs[i].max_cycles) {
#ifdef MS_PLATFORM_X86
        LOG_CRITICAL("DID NOT RECEIVE CAN MESSAGE: %u IN MAX CYCLES : %u\n", s_can_watchdogs[i].msgId,
                     s_can_watchdogs[i].max_cycles);
#else
        LOG_CRITICAL("DID NOT RECEIVE CAN MESSAGE: %lu IN MAX CYCLES : %u\n", s_can_watchdogs[i].msgId,
                     s_can_watchdogs[i].max_cycles);
#endif
      }
    }
    s_can_watchdogs[i].checked = false;
  }
  return STATUS_CODE_OK;
}
