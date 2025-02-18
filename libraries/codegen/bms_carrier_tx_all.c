#include <stdint.h>

#include "can_board_ids.h"
#include "can_codegen.h"

static CanMessage s_msg = { 
    .type = CAN_MSG_TYPE_DATA,
};
static void prv_tx_can_message(CanMessageId id, uint8_t num_bytes, uint64_t data) {
    s_msg.id.raw = id,
    s_msg.dlc = num_bytes;
    s_msg.data = data;
    s_msg.extended = (s_msg.id.msg_id >= CAN_MSG_MAX_STD_IDS);
    can_transmit(&s_msg);
}

void can_tx_all() {
    prv_tx_can_message(
        SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS,7,
        (uint64_t) g_tx_struct.battery_status_fault << 0 |
        (uint64_t) g_tx_struct.battery_status_fault_val << 16 |
        (uint64_t) g_tx_struct.battery_status_aux_batt_v << 32 |
        (uint64_t) g_tx_struct.battery_status_afe_status << 48);
    prv_tx_can_message(
        SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO,6,
        (uint64_t) g_tx_struct.battery_info_fan1 << 0 |
        (uint64_t) g_tx_struct.battery_info_fan2 << 8 |
        (uint64_t) g_tx_struct.battery_info_max_cell_v << 16 |
        (uint64_t) g_tx_struct.battery_info_min_cell_v << 32);
    prv_tx_can_message(
        SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT,8,
        (uint64_t) g_tx_struct.battery_vt_voltage << 0 |
        (uint64_t) g_tx_struct.battery_vt_current << 16 |
        (uint64_t) g_tx_struct.battery_vt_temperature << 32 |
        (uint64_t) g_tx_struct.battery_vt_batt_perc << 48);
    prv_tx_can_message(
        SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_INFO,1,
        (uint64_t) g_tx_struct.battery_relay_info_state << 0);
    prv_tx_can_message(
        SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS,8,
        (uint64_t) g_tx_struct.AFE1_status_id << 0 |
        (uint64_t) g_tx_struct.AFE1_status_temp << 8 |
        (uint64_t) g_tx_struct.AFE1_status_v1 << 16 |
        (uint64_t) g_tx_struct.AFE1_status_v2 << 32 |
        (uint64_t) g_tx_struct.AFE1_status_v3 << 48);
    prv_tx_can_message(
        SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS,8,
        (uint64_t) g_tx_struct.AFE2_status_id << 0 |
        (uint64_t) g_tx_struct.AFE2_status_temp << 8 |
        (uint64_t) g_tx_struct.AFE2_status_v1 << 16 |
        (uint64_t) g_tx_struct.AFE2_status_v2 << 32 |
        (uint64_t) g_tx_struct.AFE2_status_v3 << 48);
    prv_tx_can_message(
        SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS,8,
        (uint64_t) g_tx_struct.AFE3_status_id << 0 |
        (uint64_t) g_tx_struct.AFE3_status_temp << 8 |
        (uint64_t) g_tx_struct.AFE3_status_v1 << 16 |
        (uint64_t) g_tx_struct.AFE3_status_v2 << 32 |
        (uint64_t) g_tx_struct.AFE3_status_v3 << 48);
}