#pragma once
// Defines the CAN message type
// This is kept in a separate file to prevent cyclic dependencies
#include <stddef.h>
#include <stdint.h>

// TODO: Check these macros at a later date
#define CAN_MSG_INVALID_ID (UINT16_MAX)
#define CAN_MSG_INVALID_DEVICE (UINT16_MAX)

#define CAN_MSG_MAX_DEVICES (1 << 4)
#define CAN_MSG_MAX_IDS (1 << 24)
#define CAN_MSG_MAX_STD_IDS (1 << 11)

// TODO(ELEC-202): determine which messages are considered "critical"
#define CAN_MSG_IS_CRITICAL(msg) ((msg)->msg_id < 14)

typedef enum {
  CAN_MSG_TYPE_DATA = 0,
  NUM_CAN_MSG_TYPES,
} CanMsgType;

typedef uint32_t CanMessageId;
typedef union CanId {
  CanMessageId raw;
  struct {
    uint16_t source_id : 4;
    uint16_t type : 1; // Currently unused
    uint32_t msg_id : 24;
  };
} CanId;

typedef struct CanMessage {
  CanId id;
  CanMsgType type;
  uint8_t extended;
  size_t dlc;
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
} CanMessage;
