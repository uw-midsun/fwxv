#pragma once

#include "can_msg_defs.h"
#include "can_unpack_impl.h"


void CAN_UNPACK_BPS_HEARTBEAT(CanMessage *msg_ptr, uint8_t *status_u8_ptr){     
        can_unpack_impl_u8(msg_ptr, 1, status_u8_ptr, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY);
} 
void CAN_UNPACK_BATTERY_VT(CanMessage *msg_ptr, uint16_t *module_id_u16_ptr, uint16_t *voltage_u16_ptr, uint16_t *temperature_u16_ptr){     
        can_unpack_impl_u16(msg_ptr, 6, module_id_u16_ptr, voltage_u16_ptr, temperature_u16_ptr, CAN_UNPACK_IMPL_EMPTY);
} 
void CAN_UNPACK_BATTERY_AGGREGATE_VC(CanMessage *msg_ptr, uint32_t *voltage_u32_ptr, uint32_t *current_u32_ptr){     
        can_unpack_impl_u32(msg_ptr, 8, voltage_u32_ptr, current_u32_ptr);
} 
void CAN_UNPACK_BATTERY_FAN_STATE(CanMessage *msg_ptr, uint8_t *fan_1_u8_ptr, uint8_t *fan_2_u8_ptr, uint8_t *fan_3_u8_ptr, uint8_t *fan_4_u8_ptr, uint8_t *fan_5_u8_ptr, uint8_t *fan_6_u8_ptr, uint8_t *fan_7_u8_ptr, uint8_t *fan_8_u8_ptr){     
        can_unpack_impl_u8(msg_ptr, 8, fan_1_u8_ptr, fan_2_u8_ptr, fan_3_u8_ptr, fan_4_u8_ptr, fan_5_u8_ptr, fan_6_u8_ptr, fan_7_u8_ptr, fan_8_u8_ptr);
} 
void CAN_UNPACK_BATTERY_RELAY_STATE(CanMessage *msg_ptr, uint8_t *hv_u8_ptr, uint8_t *gnd_u8_ptr){     
        can_unpack_impl_u8(msg_ptr, 2, hv_u8_ptr, gnd_u8_ptr, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY);
} 
