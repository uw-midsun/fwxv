import struct

# Board and Message ids
SYSTEM_CAN_DEVICE_BABYDRIVER = 0
SYSTEM_CAN_DEVICE_BMS_CARRIER = 1
SYSTEM_CAN_DEVICE_CAN_COMMUNICATION = 2
SYSTEM_CAN_DEVICE_CAN_DEBUG = 3
SYSTEM_CAN_DEVICE_CENTRE_CONSOLE = 4
SYSTEM_CAN_DEVICE_CHARGER = 5
SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER = 6
SYSTEM_CAN_DEVICE_NEW_CAN = 7
SYSTEM_CAN_DEVICE_PEDAL = 8
SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION = 9
SYSTEM_CAN_DEVICE_POWER_SELECT = 10
SYSTEM_CAN_DEVICE_SOLAR_SENSE = 11
SYSTEM_CAN_DEVICE_STEERING = 12
SYSTEM_CAN_DEVICE_TELEMETRY = 13
SYSTEM_CAN_DEVICE_UV_CUTOFF = 14

NUM_SYSTEM_CAN_DEVICES = 15

SYSTEM_CAN_MESSAGE_BABYDRIVER_BABYDRIVER = (63 << 5) + SYSTEM_CAN_DEVICE_BABYDRIVER # 2016
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS = (0 << 5) + SYSTEM_CAN_DEVICE_BMS_CARRIER # 1
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT = (30 << 5) + SYSTEM_CAN_DEVICE_BMS_CARRIER # 961
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_AGGREGATE_VC = (33 << 5) + SYSTEM_CAN_DEVICE_BMS_CARRIER # 1057
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_FAN_STATE = (57 << 5) + SYSTEM_CAN_DEVICE_BMS_CARRIER # 1825
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_STATE = (58 << 5) + SYSTEM_CAN_DEVICE_BMS_CARRIER # 1857
SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_ONE_SHOT_MSG = (0 << 5) + SYSTEM_CAN_DEVICE_CAN_COMMUNICATION # 2
SYSTEM_CAN_MESSAGE_CAN_DEBUG_TEST_DEBUG = (0 << 5) + SYSTEM_CAN_DEVICE_CAN_DEBUG # 3
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_POWER_CONTROL = (1 << 5) + SYSTEM_CAN_DEVICE_CENTRE_CONSOLE # 36
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_DRIVE_OUTPUT = (2 << 5) + SYSTEM_CAN_DEVICE_CENTRE_CONSOLE # 68
SYSTEM_CAN_MESSAGE_CHARGER_REQUEST_TO_CHARGE = (48 << 5) + SYSTEM_CAN_DEVICE_CHARGER # 1541
SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_CONNECTED_STATE = (50 << 5) + SYSTEM_CAN_DEVICE_CHARGER # 1605
SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_FAULT = (53 << 5) + SYSTEM_CAN_DEVICE_CHARGER # 1701
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_CONTROLLER_VC = (30 << 5) + SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER # 966
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_VELOCITY = (31 << 5) + SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER # 998
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_STATUS = (32 << 5) + SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER # 1030
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_SINK_TEMPS = (38 << 5) + SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER # 1222
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_DSP_BOARD_TEMPS = (39 << 5) + SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER # 1254
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MC_STATUS = (40 << 5) + SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER # 1286
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_PRECHARGE_COMPLETED = (14 << 5) + SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER # 454
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1 = (0 << 5) + SYSTEM_CAN_DEVICE_NEW_CAN # 7
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG2 = (1 << 5) + SYSTEM_CAN_DEVICE_NEW_CAN # 39
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG3 = (2 << 5) + SYSTEM_CAN_DEVICE_NEW_CAN # 71
SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT = (18 << 5) + SYSTEM_CAN_DEVICE_PEDAL # 584
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_UV_CUTOFF_NOTIFICATION = (45 << 5) + SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION # 1449
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_CURRENT_MEASUREMENT = (54 << 5) + SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION # 1737
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_PD_FAULT = (62 << 5) + SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION # 1993
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_HORN_AND_LIGHTS = (46 << 5) + SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION # 1481
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_LIGHTS_SYNC = (23 << 5) + SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION # 745
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_POWER_INFO = (3 << 5) + SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION # 105
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_STATUS = (1 << 5) + SYSTEM_CAN_DEVICE_POWER_SELECT # 42
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_AUX_MEASUREMENTS = (2 << 5) + SYSTEM_CAN_DEVICE_POWER_SELECT # 74
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_DCDC_MEASUREMENTS = (3 << 5) + SYSTEM_CAN_DEVICE_POWER_SELECT # 106
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_1 = (21 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 683
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_2 = (22 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 715
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_3 = (23 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 747
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_4 = (24 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 779
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_5 = (25 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 811
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_6 = (20 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 651
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_CURRENT_SENSE = (26 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 843
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_STATUS = (27 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 875
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_TEMPS = (28 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 907
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_SOLAR_INFO = (29 << 5) + SYSTEM_CAN_DEVICE_SOLAR_SENSE # 939
SYSTEM_CAN_MESSAGE_STEERING_STEERING_INFO = (21 << 5) + SYSTEM_CAN_DEVICE_STEERING # 684
SYSTEM_CAN_MESSAGE_TELEMETRY_TEST_MSG = (0 << 5) + SYSTEM_CAN_DEVICE_TELEMETRY # 13
SYSTEM_CAN_MESSAGE_UV_CUTOFF_UV_CUTOFF_NOTIFICATION1 = (45 << 5) + SYSTEM_CAN_DEVICE_UV_CUTOFF # 1454


def pack(num, size):
    if isinstance(num, float) and (size == 32):
        return struct.pack("f", num).hex()
    elif (size == 32):
        return struct.pack("i", num).hex()
    elif (size == 16):
        return struct.pack("h", num).hex()
    elif (size == 8):
        return struct.pack("b", num).hex()


def babydriver_babydriver(id, data0, data1, data2, data3, data4, data5, data6):
    data =pack(id, 8) + pack(data0, 8) + \
		pack(data1, 8) + pack(data2, 8) + \
		pack(data3, 8) + pack(data4, 8) + \
		pack(data5, 8) + pack(data6, 8)
    return f"{SYSTEM_CAN_MESSAGE_BABYDRIVER_BABYDRIVER:3>x}#{data}"

def bms_carrier_battery_status(batt_perc, status, fault):
    data =pack(batt_perc, 16) + pack(status, 8) + \
		pack(fault, 8)
    return f"{SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS:3>x}#{data}"

def bms_carrier_battery_vt(module_id, voltage, temperature):
    data =pack(module_id, 16) + pack(voltage, 16) + \
		pack(temperature, 16)
    return f"{SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT:3>x}#{data}"

def bms_carrier_battery_aggregate_vc(voltage, current):
    data =pack(voltage, 32) + pack(current, 32)
    return f"{SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_AGGREGATE_VC:3>x}#{data}"

def bms_carrier_battery_fan_state(fan_1, fan_2, fan_3, fan_4, fan_5, fan_6, fan_7, fan_8):
    data =pack(fan_1, 8) + pack(fan_2, 8) + \
		pack(fan_3, 8) + pack(fan_4, 8) + \
		pack(fan_5, 8) + pack(fan_6, 8) + \
		pack(fan_7, 8) + pack(fan_8, 8)
    return f"{SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_FAN_STATE:3>x}#{data}"

def bms_carrier_battery_relay_state(hv, gnd):
    data =pack(hv, 8) + pack(gnd, 8)
    return f"{SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_STATE:3>x}#{data}"

def can_communication_one_shot_msg(sig1, sig2):
    data =pack(sig1, 16) + pack(sig2, 16)
    return f"{SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_ONE_SHOT_MSG:3>x}#{data}"

def can_debug_test_debug(operation, operandA, operandB, operandC):
    data =pack(operation, 8) + pack(operandA, 8) + \
		pack(operandB, 8) + pack(operandC, 8)
    return f"{SYSTEM_CAN_MESSAGE_CAN_DEBUG_TEST_DEBUG:3>x}#{data}"

def centre_console_cc_power_control(power_event, hazard_enabled):
    data =pack(power_event, 8) + pack(hazard_enabled, 8)
    return f"{SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_POWER_CONTROL:3>x}#{data}"

def centre_console_drive_output(target_velocity, drive_state, cruise_control, regen_braking, precharge):
    data =pack(target_velocity, 32) + pack(drive_state, 8) + \
		pack(cruise_control, 8) + pack(regen_braking, 8) + \
		pack(precharge, 8)
    return f"{SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_DRIVE_OUTPUT:3>x}#{data}"

def charger_request_to_charge(signal1):
    data =pack(signal1, 8)
    return f"{SYSTEM_CAN_MESSAGE_CHARGER_REQUEST_TO_CHARGE:3>x}#{data}"

def charger_charger_connected_state(is_connected):
    data =pack(is_connected, 8)
    return f"{SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_CONNECTED_STATE:3>x}#{data}"

def charger_charger_fault(fault):
    data =pack(fault, 8)
    return f"{SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_FAULT:3>x}#{data}"

def motor_controller_motor_controller_vc(mc_voltage_l, mc_current_l, mc_voltage_r, mc_current_r):
    data =pack(mc_voltage_l, 16) + pack(mc_current_l, 16) + \
		pack(mc_voltage_r, 16) + pack(mc_current_r, 16)
    return f"{SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_CONTROLLER_VC:3>x}#{data}"

def motor_controller_motor_velocity(velocity_l, velocity_r):
    data =pack(velocity_l, 16) + pack(velocity_r, 16)
    return f"{SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_VELOCITY:3>x}#{data}"

def motor_controller_motor_status(motor_status_l, motor_status_r):
    data =pack(motor_status_l, 32) + pack(motor_status_r, 32)
    return f"{SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_STATUS:3>x}#{data}"

def motor_controller_motor_sink_temps(motor_temp_l, heatsink_temp_l, motor_temp_r, heatsink_temp_r):
    data =pack(motor_temp_l, 16) + pack(heatsink_temp_l, 16) + \
		pack(motor_temp_r, 16) + pack(heatsink_temp_r, 16)
    return f"{SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_SINK_TEMPS:3>x}#{data}"

def motor_controller_dsp_board_temps(dsp_temp_l, dsp_temp_r):
    data =pack(dsp_temp_l, 32) + pack(dsp_temp_r, 32)
    return f"{SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_DSP_BOARD_TEMPS:3>x}#{data}"

def motor_controller_mc_status(limit_bitset_l, error_bitset_l, limit_bitset_r, error_bitset_r, board_fault_bitset, overtemp_bitset, precharge_status):
    data =pack(limit_bitset_l, 8) + pack(error_bitset_l, 8) + \
		pack(limit_bitset_r, 8) + pack(error_bitset_r, 8) + \
		pack(board_fault_bitset, 8) + pack(overtemp_bitset, 8) + \
		pack(precharge_status, 8)
    return f"{SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MC_STATUS:3>x}#{data}"

def motor_controller_precharge_completed(notification):
    data =pack(notification, 8)
    return f"{SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_PRECHARGE_COMPLETED:3>x}#{data}"

def new_can_transmit_msg1(status):
    data =pack(status, 8)
    return f"{SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1:3>x}#{data}"

def new_can_transmit_msg2(signal, signal2):
    data =pack(signal, 8) + pack(signal2, 8)
    return f"{SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG2:3>x}#{data}"

def new_can_transmit_msg3(help):
    data =pack(help, 8)
    return f"{SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG3:3>x}#{data}"

def pedal_pedal_output(throttle_output, brake_output):
    data =pack(throttle_output, 32) + pack(brake_output, 32)
    return f"{SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT:3>x}#{data}"

def power_distribution_uv_cutoff_notification(signal1):
    data =pack(signal1, 8)
    return f"{SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_UV_CUTOFF_NOTIFICATION:3>x}#{data}"

def power_distribution_current_measurement(current_id, current):
    data =pack(current_id, 16) + pack(current, 16)
    return f"{SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_CURRENT_MEASUREMENT:3>x}#{data}"

def power_distribution_pd_fault(fault_data, faulting_output):
    data =pack(fault_data, 16) + pack(faulting_output, 16)
    return f"{SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_PD_FAULT:3>x}#{data}"

def power_distribution_horn_and_lights(horn_state, lights_state):
    data =pack(horn_state, 8) + pack(lights_state, 8)
    return f"{SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_HORN_AND_LIGHTS:3>x}#{data}"

def power_distribution_lights_sync(signal1):
    data =pack(signal1, 8)
    return f"{SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_LIGHTS_SYNC:3>x}#{data}"

def power_distribution_power_info(power_state, pd_fault):
    data =pack(power_state, 8) + pack(pd_fault, 8)
    return f"{SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_POWER_INFO:3>x}#{data}"

def power_select_power_select_status(status, fault):
    data =pack(status, 8) + pack(fault, 8)
    return f"{SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_STATUS:3>x}#{data}"

def power_select_power_select_aux_measurements(aux_voltage, aux_current, aux_temp, power_supply_current):
    data =pack(aux_voltage, 16) + pack(aux_current, 16) + \
		pack(aux_temp, 16) + pack(power_supply_current, 16)
    return f"{SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_AUX_MEASUREMENTS:3>x}#{data}"

def power_select_power_select_dcdc_measurements(dcdc_voltage, dcdc_current, dcdc_temp, power_supply_voltage):
    data =pack(dcdc_voltage, 16) + pack(dcdc_current, 16) + \
		pack(dcdc_temp, 16) + pack(power_supply_voltage, 16)
    return f"{SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_DCDC_MEASUREMENTS:3>x}#{data}"

def solar_sense_mppt_1(current, voltage, pwm, status):
    data =pack(current, 16) + pack(voltage, 16) + \
		pack(pwm, 16) + pack(status, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_1:3>x}#{data}"

def solar_sense_mppt_2(current, voltage, pwm, status):
    data =pack(current, 16) + pack(voltage, 16) + \
		pack(pwm, 16) + pack(status, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_2:3>x}#{data}"

def solar_sense_mppt_3(current, voltage, pwm, status):
    data =pack(current, 16) + pack(voltage, 16) + \
		pack(pwm, 16) + pack(status, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_3:3>x}#{data}"

def solar_sense_mppt_4(current, voltage, pwm, status):
    data =pack(current, 16) + pack(voltage, 16) + \
		pack(pwm, 16) + pack(status, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_4:3>x}#{data}"

def solar_sense_mppt_5(current, voltage, pwm, status):
    data =pack(current, 16) + pack(voltage, 16) + \
		pack(pwm, 16) + pack(status, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_5:3>x}#{data}"

def solar_sense_mppt_6(current, voltage, pwm, status):
    data =pack(current, 16) + pack(voltage, 16) + \
		pack(pwm, 16) + pack(status, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_6:3>x}#{data}"

def solar_sense_current_sense(current, voltage, relay_status):
    data =pack(current, 16) + pack(voltage, 16) + \
		pack(relay_status, 8)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_CURRENT_SENSE:3>x}#{data}"

def solar_sense_thermal_status(overtemp, fullspeed, fan_fail, temp_1, temp_2):
    data =pack(overtemp, 8) + pack(fullspeed, 8) + \
		pack(fan_fail, 8) + pack(temp_1, 16) + \
		pack(temp_2, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_STATUS:3>x}#{data}"

def solar_sense_thermal_temps(temp_3, temp_4, temp_5, temp_6):
    data =pack(temp_3, 16) + pack(temp_4, 16) + \
		pack(temp_5, 16) + pack(temp_6, 16)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_TEMPS:3>x}#{data}"

def solar_sense_solar_info(solar_fault):
    data =pack(solar_fault, 8)
    return f"{SYSTEM_CAN_MESSAGE_SOLAR_SENSE_SOLAR_INFO:3>x}#{data}"

def steering_steering_info(input_cc, input_lights):
    data =pack(input_cc, 8) + pack(input_lights, 8)
    return f"{SYSTEM_CAN_MESSAGE_STEERING_STEERING_INFO:3>x}#{data}"

def telemetry_test_msg(test):
    data =pack(test, 8)
    return f"{SYSTEM_CAN_MESSAGE_TELEMETRY_TEST_MSG:3>x}#{data}"

def uv_cutoff_uv_cutoff_notification1(signal1):
    data =pack(signal1, 8)
    return f"{SYSTEM_CAN_MESSAGE_UV_CUTOFF_UV_CUTOFF_NOTIFICATION1:3>x}#{data}"
