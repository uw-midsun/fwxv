import struct
import subprocess

# Board and Message ids
SYSTEM_CAN_DEVICE_BABYDRIVER = 0
SYSTEM_CAN_DEVICE_BMS_CARRIER = 1
SYSTEM_CAN_DEVICE_CENTRE_CONSOLE = 2
SYSTEM_CAN_DEVICE_CHARGER = 3
SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER = 4
SYSTEM_CAN_DEVICE_PEDAL = 5
SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT = 6
SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR = 7
SYSTEM_CAN_DEVICE_POWER_SELECT = 8
SYSTEM_CAN_DEVICE_SOLAR_5_MPPTS = 9
SYSTEM_CAN_DEVICE_SOLAR_6_MPPTS = 10
SYSTEM_CAN_DEVICE_STEERING = 11
SYSTEM_CAN_DEVICE_UV_CUTOFF = 12
SYSTEM_CAN_DEVICE_SOLAR_SENSE = 13
SYSTEM_CAN_DEVICE_NEW_CAN = 14
SYSTEM_CAN_DEVICE_CAN_COMMUNICATION = 15
SYSTEM_CAN_DEVICE_CAN_DEBUG = 16
SYSTEM_CAN_DEVICE_TELEMETRY = 17

SYSTEM_CAN_MESSAGE_BABYDRIVER_BABYDRIVER = 2016,
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BPS_HEARTBEAT = 1,
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT = 961,
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_AGGREGATE_VC = 1057,
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_FAN_STATE = 1825,
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_STATE = 1857,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_SET_BMS_POWER = 130,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_SET_POWER_STATE = 162,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_SET_RELAY_STATES = 34,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_POWER_ON_MAIN_SEQUENCE = 194,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_POWER_OFF_MAIN_SEQUENCE = 226,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_POWER_ON_AUX_SEQUENCE = 258,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_DRIVE_OUTPUT = 290,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_SET_EBRAKE_STATE = 322,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_READY_TO_DRIVE = 418,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_BEGIN_PRECHARGE = 898,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_HAZARD = 1026,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_DISCHARGE_PRECHARGE = 994,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_STATE_TRANSITION_FAULT = 1090,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_ALLOW_CHARGING = 1570,
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_POWER_INFO = 1506,
SYSTEM_CAN_MESSAGE_CHARGER_REQUEST_TO_CHARGE = 1539,
SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_CONNECTED_STATE = 1603,
SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_FAULT = 1699,
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_CONTROLLER_VC = 1124,
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_VELOCITY = 1156,
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_SINK_TEMPS = 1220,
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_DSP_BOARD_TEMPS = 1252,
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MC_STATUS = 1284,
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_PRECHARGE_COMPLETED = 516,
SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT = 581,
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_FRONT_UV_CUTOFF_NOTIFICATION = 1446,
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_FRONT_FRONT_CURRENT_MEASUREMENT = 1734,
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_FRONT_FRONT_PD_FAULT = 1990,
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_FRONT_HORN_AND_LIGHTS = 1478,
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_REAR_LIGHTS_SYNC = 743,
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_REAR_REAR_CURRENT_MEASUREMENT = 1767,
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_REAR_REAR_PD_FAULT = 1959,
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_STATUS = 40,
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_AUX_MEASUREMENTS = 72,
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_DCDC_MEASUREMENTS = 104,
SYSTEM_CAN_MESSAGE_SOLAR_5_MPPTS_RELAY_OPEN_OK_5_MPPTS = 457,
SYSTEM_CAN_MESSAGE_SOLAR_5_MPPTS_SOLAR_DATA_5_MPPTS = 1897,
SYSTEM_CAN_MESSAGE_SOLAR_5_MPPTS_SOLAR_FAULT_5_MPPTS = 1929,
SYSTEM_CAN_MESSAGE_SOLAR_6_MPPTS_RELAY_OPEN_OK_6_MPPTS = 490,
SYSTEM_CAN_MESSAGE_SOLAR_6_MPPTS_SOLAR_DATA_6_MPPTS = 1642,
SYSTEM_CAN_MESSAGE_SOLAR_6_MPPTS_SOLAR_FAULT_6_MPPTS = 1674,
SYSTEM_CAN_MESSAGE_STEERING_STEERING_INFO = 683,
SYSTEM_CAN_MESSAGE_UV_CUTOFF_UV_CUTOFF_NOTIFICATION = 1452,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_1 = 685,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_2 = 717,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_3 = 749,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_4 = 781,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_5 = 813,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_6 = 653,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_CURRENT_SENSE = 845,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_STATUS = 877,
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_TEMPS = 909,
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1 = 14,
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG2 = 46,
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG3 = 78,
SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_ONE_SHOT_MSG = 15,
SYSTEM_CAN_MESSAGE_CAN_DEBUG_TEST_DEBUG = 16,
SYSTEM_CAN_MESSAGE_TELEMETRY_TEST_MSG = 17,


# Send
def send_message(id, data):
  cmd = f"cansend can0 {id:x}#{data:x}"
  subprocess.run(cmd)

def pack(num, size):
  if isinstance(num, float) and (size == 32):
    return struct.pack("f", num).hex()
  elif (size == 32):
    return struct.pack("i", num).hex()
  elif (size == 16):
    return struct.pack("h", num).hex()
  elif (size == 8):
    return struct.pack("b", num).hex()
def send_message_babydriver_babydriver(id, data0, data1, data2, data3, data4, data5, data6):
    send_message(2016,pack(id, 8)+pack(data0, 8)+pack(data1, 8)+pack(data2, 8)+pack(data3, 8)+pack(data4, 8)+pack(data5, 8)+pack(data6, 8))

def send_message_bms_carrier_bps_heartbeat(status):
    send_message(1,pack(status, 8))

def send_message_bms_carrier_battery_vt(module_id, voltage, temperature, batt_perc):
    send_message(961,pack(module_id, 16)+pack(voltage, 16)+pack(temperature, 16)+pack(batt_perc, 16))

def send_message_bms_carrier_battery_aggregate_vc(voltage, current):
    send_message(1057,pack(voltage, 32)+pack(current, 32))

def send_message_bms_carrier_battery_fan_state(fan_1, fan_2, fan_3, fan_4, fan_5, fan_6, fan_7, fan_8):
    send_message(1825,pack(fan_1, 8)+pack(fan_2, 8)+pack(fan_3, 8)+pack(fan_4, 8)+pack(fan_5, 8)+pack(fan_6, 8)+pack(fan_7, 8)+pack(fan_8, 8))

def send_message_bms_carrier_battery_relay_state(hv, gnd):
    send_message(1857,pack(hv, 8)+pack(gnd, 8))

def send_message_centre_console_set_bms_power(bms_power_on_notification):
    send_message(130,pack(bms_power_on_notification, 8))

def send_message_centre_console_set_power_state(turn_on_everything_notification):
    send_message(162,pack(turn_on_everything_notification, 8))

def send_message_centre_console_set_relay_states(relay_mask, relay_state):
    send_message(34,pack(relay_mask, 16)+pack(relay_state, 16))

def send_message_centre_console_power_on_main_sequence(sequence):
    send_message(194,pack(sequence, 16))

def send_message_centre_console_power_off_main_sequence(sequence):
    send_message(226,pack(sequence, 16))

def send_message_centre_console_power_on_aux_sequence(sequence):
    send_message(258,pack(sequence, 16))

def send_message_centre_console_drive_output(target_velocity, drive_state, cruise_control, regen_braking, precharge):
    send_message(290,pack(target_velocity, 32)+pack(drive_state, 8)+pack(cruise_control, 8)+pack(regen_braking, 8)+pack(precharge, 8))

def send_message_centre_console_set_ebrake_state(ebrake_state):
    send_message(322,pack(ebrake_state, 8))

def send_message_centre_console_ready_to_drive(ready_state):
    send_message(418,pack(ready_state, 8))

def send_message_centre_console_begin_precharge(signal1):
    send_message(898,pack(signal1, 8))

def send_message_centre_console_hazard(state):
    send_message(1026,pack(state, 8))

def send_message_centre_console_discharge_precharge(signal1):
    send_message(994,pack(signal1, 8))

def send_message_centre_console_state_transition_fault(state_machine, fault_reason):
    send_message(1090,pack(state_machine, 16)+pack(fault_reason, 16))

def send_message_centre_console_allow_charging(signal1):
    send_message(1570,pack(signal1, 8))

def send_message_centre_console_power_info(hazard_state, power_state):
    send_message(1506,pack(hazard_state, 8)+pack(power_state, 8))

def send_message_charger_request_to_charge(signal1):
    send_message(1539,pack(signal1, 8))

def send_message_charger_charger_connected_state(is_connected):
    send_message(1603,pack(is_connected, 8))

def send_message_charger_charger_fault(fault):
    send_message(1699,pack(fault, 8))

def send_message_motor_controller_motor_controller_vc(mc_voltage_l, mc_current_l, mc_voltage_r, mc_current_r):
    send_message(1124,pack(mc_voltage_l, 16)+pack(mc_current_l, 16)+pack(mc_voltage_r, 16)+pack(mc_current_r, 16))

def send_message_motor_controller_motor_velocity(velocity_l, velocity_r):
    send_message(1156,pack(velocity_l, 16)+pack(velocity_r, 16))

def send_message_motor_controller_motor_sink_temps(motor_temp_l, heatsink_temp_l, motor_temp_r, heatsink_temp_r):
    send_message(1220,pack(motor_temp_l, 16)+pack(heatsink_temp_l, 16)+pack(motor_temp_r, 16)+pack(heatsink_temp_r, 16))

def send_message_motor_controller_dsp_board_temps(dsp_temp_l, dsp_temp_r):
    send_message(1252,pack(dsp_temp_l, 16)+pack(dsp_temp_r, 16))

def send_message_motor_controller_mc_status(limit_bitset_l, error_bitset_l, limit_bitset_r, error_bitset_r, board_fault_bitset, overtemp_bitset, precharge_status):
    send_message(1284,pack(limit_bitset_l, 8)+pack(error_bitset_l, 8)+pack(limit_bitset_r, 8)+pack(error_bitset_r, 8)+pack(board_fault_bitset, 8)+pack(overtemp_bitset, 8)+pack(precharge_status, 8))

def send_message_motor_controller_precharge_completed(notification):
    send_message(516,pack(notification, 8))

def send_message_pedal_pedal_output(throttle_output, brake_output):
    send_message(581,pack(throttle_output, 32)+pack(brake_output, 32))

def send_message_power_distribution_front_uv_cutoff_notification(signal1):
    send_message(1446,pack(signal1, 8))

def send_message_power_distribution_front_front_current_measurement(current_id, current):
    send_message(1734,pack(current_id, 16)+pack(current, 16))

def send_message_power_distribution_front_front_pd_fault(fault_data, faulting_output):
    send_message(1990,pack(fault_data, 16)+pack(faulting_output, 16))

def send_message_power_distribution_front_horn_and_lights(horn_state, lights_state):
    send_message(1478,pack(horn_state, 8)+pack(lights_state, 8))

def send_message_power_distribution_rear_lights_sync(signal1):
    send_message(743,pack(signal1, 8))

def send_message_power_distribution_rear_rear_current_measurement(current_id, current):
    send_message(1767,pack(current_id, 16)+pack(current, 16))

def send_message_power_distribution_rear_rear_pd_fault(fault_data, enclosure_temp_data, dcdc_temp_data, faulting_output):
    send_message(1959,pack(fault_data, 16)+pack(enclosure_temp_data, 16)+pack(dcdc_temp_data, 16)+pack(faulting_output, 16))

def send_message_power_select_power_select_status(status, fault):
    send_message(40,pack(status, 8)+pack(fault, 8))

def send_message_power_select_power_select_aux_measurements(aux_voltage, aux_current, aux_temp, power_supply_current):
    send_message(72,pack(aux_voltage, 16)+pack(aux_current, 16)+pack(aux_temp, 16)+pack(power_supply_current, 16))

def send_message_power_select_power_select_dcdc_measurements(dcdc_voltage, dcdc_current, dcdc_temp, power_supply_voltage):
    send_message(104,pack(dcdc_voltage, 16)+pack(dcdc_current, 16)+pack(dcdc_temp, 16)+pack(power_supply_voltage, 16))

def send_message_solar_5_mppts_relay_open_ok_5_mppts(signal1):
    send_message(457,pack(signal1, 8))

def send_message_solar_5_mppts_solar_data_5_mppts(data_point_type, data_value):
    send_message(1897,pack(data_point_type, 32)+pack(data_value, 32))

def send_message_solar_5_mppts_solar_fault_5_mppts(fault, fault_data):
    send_message(1929,pack(fault, 8)+pack(fault_data, 8))

def send_message_solar_6_mppts_relay_open_ok_6_mppts(signal1):
    send_message(490,pack(signal1, 8))

def send_message_solar_6_mppts_solar_data_6_mppts(data_point_type, data_value):
    send_message(1642,pack(data_point_type, 32)+pack(data_value, 32))

def send_message_solar_6_mppts_solar_fault_6_mppts(fault, fault_data):
    send_message(1674,pack(fault, 8)+pack(fault_data, 8))

def send_message_steering_steering_info(digital_input, analog_input):
    send_message(683,pack(digital_input, 8)+pack(analog_input, 8))

def send_message_uv_cutoff_uv_cutoff_notification(signal1):
    send_message(1452,pack(signal1, 8))

def send_message_solar_sense_mppt_1(current, voltage, pwm, status):
    send_message(685,pack(current, 16)+pack(voltage, 16)+pack(pwm, 16)+pack(status, 16))

def send_message_solar_sense_mppt_2(current, voltage, pwm, status):
    send_message(717,pack(current, 16)+pack(voltage, 16)+pack(pwm, 16)+pack(status, 16))

def send_message_solar_sense_mppt_3(current, voltage, pwm, status):
    send_message(749,pack(current, 16)+pack(voltage, 16)+pack(pwm, 16)+pack(status, 16))

def send_message_solar_sense_mppt_4(current, voltage, pwm, status):
    send_message(781,pack(current, 16)+pack(voltage, 16)+pack(pwm, 16)+pack(status, 16))

def send_message_solar_sense_mppt_5(current, voltage, pwm, status):
    send_message(813,pack(current, 16)+pack(voltage, 16)+pack(pwm, 16)+pack(status, 16))

def send_message_solar_sense_mppt_6(current, voltage, pwm, status):
    send_message(653,pack(current, 16)+pack(voltage, 16)+pack(pwm, 16)+pack(status, 16))

def send_message_solar_sense_current_sense(current, voltage, relay_status):
    send_message(845,pack(current, 16)+pack(voltage, 16)+pack(relay_status, 8))

def send_message_solar_sense_thermal_status(overtemp, fullspeed, fan_fail, temp_1, temp_2):
    send_message(877,pack(overtemp, 8)+pack(fullspeed, 8)+pack(fan_fail, 8)+pack(temp_1, 16)+pack(temp_2, 16))

def send_message_solar_sense_thermal_temps(temp_3, temp_4, temp_5, temp_6):
    send_message(909,pack(temp_3, 16)+pack(temp_4, 16)+pack(temp_5, 16)+pack(temp_6, 16))

def send_message_new_can_transmit_msg1(status):
    send_message(14,pack(status, 8))

def send_message_new_can_transmit_msg2(signal, signal2):
    send_message(46,pack(signal, 8)+pack(signal2, 8))

def send_message_new_can_transmit_msg3(help):
    send_message(78,pack(help, 8))

def send_message_can_communication_one_shot_msg(sig1, sig2):
    send_message(15,pack(sig1, 16)+pack(sig2, 16))

def send_message_can_debug_test_debug(operation, operandA, operandB, operandC):
    send_message(16,pack(operation, 8)+pack(operandA, 8)+pack(operandB, 8)+pack(operandC, 8))

def send_message_telemetry_test_msg(test):
    send_message(17,pack(test, 8))








