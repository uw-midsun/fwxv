class CanSimulatorDataCache:
    def __init__(self):
        self.data = {

            "telemetry_telemetry": {
                # telemetry cache definitions
                "data": 0,
            },

            "motor_controller_motor_controller_vc": {
                # motor_controller_vc cache definitions
                "mc_voltage_l": 0,
                "mc_current_l": 0,
                "mc_voltage_r": 0,
                "mc_current_r": 0,
            },

            "motor_controller_motor_velocity": {
                # motor_velocity cache definitions
                "velocity_l": 0,
                "velocity_r": 0,
                "brakes_enabled": 0,
            },

            "motor_controller_motor_sink_temps": {
                # motor_sink_temps cache definitions
                "motor_temp_l": 0,
                "heatsink_temp_l": 0,
                "motor_temp_r": 0,
                "heatsink_temp_r": 0,
            },

            "motor_controller_dsp_board_temps": {
                # dsp_board_temps cache definitions
                "dsp_temp_l": 0,
                "dsp_temp_r": 0,
            },

            "motor_controller_mc_status": {
                # mc_status cache definitions
                "limit_bitset_l": 0,
                "error_bitset_l": 0,
                "limit_bitset_r": 0,
                "error_bitset_r": 0,
                "board_fault_bitset": 0,
                "overtemp_bitset": 0,
                "precharge_status": 0,
            },

            "bootloader_bootloader_start": {
                # bootloader_start cache definitions
                "data0": 0,
                "data1": 0,
                "data2": 0,
                "data3": 0,
                "data4": 0,
                "data5": 0,
                "data6": 0,
                "data7": 0,
            },

            "bootloader_bootloader_data": {
                # bootloader_data cache definitions
                "data0": 0,
                "data1": 0,
                "data2": 0,
                "data3": 0,
                "data4": 0,
                "data5": 0,
                "data6": 0,
                "data7": 0,
            },

            "can_communication_one_shot_msg": {
                # one_shot_msg cache definitions
                "sig1": 0,
                "sig2": 0,
            },

            "can_debug_test_debug": {
                # test_debug cache definitions
                "operation": 0,
                "operandA": 0,
                "operandB": 0,
                "operandC": 0,
            },

            "new_can_transmit_msg1": {
                # transmit_msg1 cache definitions
                "status": 0,
            },

            "new_can_transmit_msg2": {
                # transmit_msg2 cache definitions
                "signal": 0,
                "signal2": 0,
            },

            "new_can_transmit_msg3": {
                # transmit_msg3 cache definitions
                "help": 0,
            },

            "bms_carrier_battery_status": {
                # battery_status cache definitions
                "fault": 0,
                "fault_val": 0,
                "aux_batt_v": 0,
                "afe_status": 0,
            },

            "bms_carrier_battery_info": {
                # battery_info cache definitions
                "fan1": 0,
                "fan2": 0,
                "max_cell_v": 0,
                "min_cell_v": 0,
            },

            "bms_carrier_battery_vt": {
                # battery_vt cache definitions
                "voltage": 0,
                "current": 0,
                "temperature": 0,
                "batt_perc": 0,
            },

            "bms_carrier_battery_relay_info": {
                # battery_relay_info cache definitions
                "state": 0,
            },

            "bms_carrier_afe1_status": {
                # afe1_status cache definitions
                "id": 0,
                "temp": 0,
                "v1": 0,
                "v2": 0,
                "v3": 0,
            },

            "bms_carrier_afe2_status": {
                # afe2_status cache definitions
                "id": 0,
                "temp": 0,
                "v1": 0,
                "v2": 0,
                "v3": 0,
            },

            "bms_carrier_afe3_status": {
                # afe3_status cache definitions
                "id": 0,
                "temp": 0,
                "v1": 0,
                "v2": 0,
                "v3": 0,
            },

            "centre_console_cc_info": {
                # cc_info cache definitions
                "target_velocity": 0,
                "drive_state": 0,
                "cruise_control": 0,
                "regen_braking": 0,
                "hazard_enabled": 0,
            },

            "centre_console_cc_pedal": {
                # cc_pedal cache definitions
                "throttle_output": 0,
                "brake_output": 0,
            },

            "centre_console_cc_steering": {
                # cc_steering cache definitions
                "input_cc": 0,
                "input_lights": 0,
            },

            "centre_console_cc_regen": {
                # cc_regen cache definitions
                "percent": 0,
            },

            "power_distribution_current_measurement_1": {
                # current_measurement_1 cache definitions
                "current_id_1": 0,
                "current_1": 0,
                "current_id_2": 0,
                "current_2": 0,
            },

            "power_distribution_current_measurement_2": {
                # current_measurement_2 cache definitions
                "current_id_1": 0,
                "current_1": 0,
                "current_id_2": 0,
                "current_2": 0,
            },

            "power_distribution_pd_status": {
                # pd_status cache definitions
                "power_state": 0,
                "fault_bitset": 0,
                "bps_persist": 0,
                "bps_persist_val": 0,
            }

        }

    def get(self, message_name, parameter):
        return self.data.get(message_name, {}).get(parameter, 0)
    
    def set(self, message_name, parameter, value):
        if message_name in self.data and parameter in self.data[message_name]:
            self.data[message_name][parameter] = value
            return True
        return False