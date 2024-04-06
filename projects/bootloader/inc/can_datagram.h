#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "status.h"

#define DGRAM_MAX_DATA_SIZE 2048
#define DGRAM_MAX_MSG_SIZE 8

#define DGRAM_TYPE_SIZE_BYTES 1
#define CRC_SIZE_BYTES 4
#define NODE_IDS_SIZE_BYTES 2
#define DATA_LEN_SIZE_BYTES 2

#define LITTLE_ENDIANIZE(value, outbuf)                 \
  for (size_t byte = 0; byte < sizeof(value); byte++) { \
    outbuf[byte] = value >> (8 * byte) & 0xFF;          \
  }

// Callback used to tx a datagram message, used to handle all CAN transmission
// Called in each state as data becomes ready to be transmitted
typedef StatusCode (*CanDatagramTxCb)(uint8_t *data, size_t len, bool start_message);

// Optional callbacks called on tx/rx completion and error
typedef void (*CanDatagramExitCb)(void);

typedef struct CanDatagram {
  uint8_t dgram_type;
  uint32_t crc32;
  uint16_t node_ids;
  uint16_t data_len;
  uint8_t *data;
} CanDatagram;

// Tx Config - Data and NodeID buffers must remain available
// for entirety of datagram execution
typedef struct CanDatagramTxConfig {
  uint8_t dgram_type;
  uint8_t *node_ids_list;
  uint8_t node_ids_length;
  uint16_t data_len;
  uint8_t *data;
  // Mandatory callback to handle transmission
  CanDatagramTxCb tx_cb;
  // Optional callback - called on tx completion
  CanDatagramExitCb tx_cmpl_cb;
} CanDatagramTxConfig;

typedef struct CanDatagramRxConfig {
  // These parameters must be passed
  uint16_t node_ids;
  uint8_t *data;
  // These parameters will be set by rcv'd data
  uint8_t dgram_type;
  uint8_t data_len;
  uint32_t crc32;
  // Optional callback - called on rx completion
  CanDatagramExitCb rx_cmpl_cb;
} CanDatagramRxConfig;

typedef enum {
  DATAGRAM_START = 0,
  DATAGRAM_DATA,
} CanDatagramTypeId;

typedef enum {
  DATAGRAM_STATUS_ACTIVE = 0,
  DATAGRAM_STATUS_IDLE,
  DATAGRAM_STATUS_TX_COMPLETE,
  DATAGRAM_STATUS_RX_COMPLETE,
  DATAGRAM_STATUS_ERROR,
  NUM_DATAGRAM_STATUSES,
} CanDatagramStatus;

typedef struct CanDatagramStorage {
  CanDatagram dgram;
  CanDatagramTxCb tx_cb;
  CanDatagramStatus status;
  CanDatagramRxConfig *rx_info;

  CanDatagramExitCb rx_cmpl_cb;
  CanDatagramExitCb tx_cmpl_cb;
  CanDatagramExitCb error_cb;

  size_t tx_bytes_sent;
  bool rx_listener_enabled;
  bool start_received;
  uint8_t node_id;
} CanDatagramStorage;

// Initializes a can datagram instance and prepares for transmitting or receiving
StatusCode can_datagram_init();

// Called in the rx handler for datagram messages to process sequential messages
StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message);

// Called after initialization to start txing datagram messages
StatusCode can_datagram_start_tx(CanDatagramTxConfig *config);

// Used to start listening for/processing rx can datagrams
// Will continue to listen unless explicitly told to stop
StatusCode can_datagram_start_listener(CanDatagramRxConfig *config);

// Returns true if the datagram is complete (all data were sent/read)
CanDatagramStatus can_datagram_get_status(void);