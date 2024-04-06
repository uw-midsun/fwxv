#include "can_datagram.h"
#include "log.h"

static CanDatagramStorage s_store;

static_assert((sizeof(s_store.dgram.dgram_type) == DGRAM_TYPE_SIZE_BYTES),
              "Dgram type is wrong size!");
static_assert((sizeof(s_store.dgram.crc32) == CRC_SIZE_BYTES), "CRC is wrong size!");
static_assert((sizeof(s_store.dgram.node_ids) == NODE_IDS_SIZE_BYTES), "Node_Ids is wrong size!");
static_assert((sizeof(s_store.dgram.data_len) == DATA_LEN_SIZE_BYTES), "Data_len is wrong size!");

static uint8_t s_buffer[DGRAM_MAX_DATA_SIZE];
static uint8_t s_can_buffer[DGRAM_MAX_MSG_SIZE];

static uint32_t prv_can_datagram_compute_crc32() {
    // UNIMPLEMENTED NEW MEMBER TASK (crc32 calculation)

    // nodes_crc32 = nodes_raw calculate crc32
    // data_crc32 = data calculate crc32

    // datagram_id + len nodes_raw + nodes_crc32 + len data + data_crc32
    return STATUS_CODE_OK;
}

static uint16_t prv_convert_node_ids(uint8_t nodes_list[], uint8_t nodes_length) {
    uint16_t out_val = 0;
    for (int i = 0; i <= nodes_length; i++) {
        out_val |= (1 << (nodes_list[i] - 1));
    }
    return out_val;
} 

StatusCode can_datagram_init() {
    // Reset the store
    s_store.tx_cb = NULL;
    s_store.rx_info = NULL;
    s_store.tx_bytes_sent = 0;
    s_store.rx_listener_enabled = false;
    s_store.node_id = 0;
    s_store.status = DATAGRAM_STATUS_IDLE;
    return STATUS_CODE_OK;
}

StatusCode can_datagram_start_tx(CanDatagramTxConfig *config) {
    // Mutex locks for s_store status?
    if (s_store.status == DATAGRAM_STATUS_ACTIVE) {
        return STATUS_CODE_UNREACHABLE;
    }
    if (config->tx_cb == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }
    if (config->data_len > DGRAM_MAX_DATA_SIZE) {
        return STATUS_CODE_INVALID_ARGS;
    }

    CanDatagram *dgram = &s_store.dgram;
    dgram->dgram_type = config->dgram_type;
    dgram->node_ids = prv_convert_node_ids(config->node_ids_list, config->node_ids_length);
    dgram->data_len = config->data_len;
    dgram->data = config->data;
    dgram->crc32 = prv_can_datagram_compute_crc32();

    LOG_DEBUG("dgram->node_ids %d\n", dgram->node_ids);
    s_store.tx_cb = config->tx_cb;
    s_store.tx_cmpl_cb = config->tx_cmpl_cb;

    s_store.status = DATAGRAM_STATUS_ACTIVE;

    return STATUS_CODE_OK;
}

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start) {
    if (len > DGRAM_MAX_MSG_SIZE) {
        return STATUS_CODE_OUT_OF_RANGE;
    }

    if (!s_store.rx_listener_enabled) {
        return STATUS_CODE_UNINITIALIZED;
    }

    return STATUS_CODE_OK;
}

CanDatagramStatus can_datagram_get_status() {
    return s_store.status;
}