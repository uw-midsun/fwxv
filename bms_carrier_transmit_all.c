#include "bms_carrier_pack_msg.h"
#include "can_codegen.h"

void can_transmit_all() {
  CanMessage msg = { 0 };

  can_pack_bps_heartbeat(&msg, &g_transmit_struct.bps_heartbeat_status);
  can_transmit(&msg, NULL);

  can_pack_battery_vt(&msg, &g_transmit_struct.battery_vt_module_id, 
                                                        &g_transmit_struct.battery_vt_voltage, 
                                                        &g_transmit_struct.battery_vt_temperature);
  can_transmit(&msg, NULL);

  can_pack_battery_aggregate_vc(&msg, &g_transmit_struct.battery_aggregate_vc_voltage, 
                                                                          &g_transmit_struct.battery_aggregate_vc_current);
  can_transmit(&msg, NULL);

  can_pack_battery_fan_state(&msg, &g_transmit_struct.battery_fan_state_fan_1, 
                                                                  &g_transmit_struct.battery_fan_state_fan_2, 
                                                                  &g_transmit_struct.battery_fan_state_fan_3, 
                                                                  &g_transmit_struct.battery_fan_state_fan_4, 
                                                                  &g_transmit_struct.battery_fan_state_fan_5, 
                                                                  &g_transmit_struct.battery_fan_state_fan_6, 
                                                                  &g_transmit_struct.battery_fan_state_fan_7, 
                                                                  &g_transmit_struct.battery_fan_state_fan_8);
  can_transmit(&msg, NULL);

  can_pack_battery_relay_state(&msg, &g_transmit_struct.battery_relay_state_hv, 
                                                                   &g_transmit_struct.battery_relay_state_gnd);
  can_transmit(&msg, NULL);

  
}