#pragma once

#include "can_msg_defs.h"
#include "can_pack_impl.h"



void can_pack_bps_heartbeat(CanMessage *msg_ptr, uint8_t status_u8){
    can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, 
}

void can_pack_battery_vt(CanMessage *msg_ptr, uint16_t module_id_u16, uint16_t voltage_u16, uint16_t temperature_u16){
    can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BATTERY_VT, 
}

void can_pack_battery_aggregate_vc(CanMessage *msg_ptr, uint32_t voltage_u32, uint32_t current_u32){
    can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC, 
}

void can_pack_battery_fan_state(CanMessage *msg_ptr, uint8_t fan_1_u8, uint8_t fan_2_u8, uint8_t fan_3_u8, uint8_t fan_4_u8, uint8_t fan_5_u8, uint8_t fan_6_u8, uint8_t fan_7_u8, uint8_t fan_8_u8){
    can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BATTERY_FAN_STATE, 
}

void can_pack_battery_relay_state(CanMessage *msg_ptr, uint8_t hv_u8, uint8_t gnd_u8){
    can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BATTERY_RELAY_STATE, 
}
