import struct
import subprocess
import time 
import threading

SYSTEM_CAN_MESSAGE_BABYDRIVER_BABYDRIVER = 2016
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS = 1
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT = 961
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_AGGREGATE_VC = 1057
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_FAN_STATE = 1825
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_INFO = 1857
SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_ONE_SHOT_MSG = 2
SYSTEM_CAN_MESSAGE_CAN_DEBUG_TEST_DEBUG = 3
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_POWER_CONTROL = 36
SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_DRIVE_OUTPUT = 68
SYSTEM_CAN_MESSAGE_CHARGER_REQUEST_TO_CHARGE = 1541
SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_CONNECTED_STATE = 1605
SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_FAULT = 1701
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_CONTROLLER_VC = 966
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_VELOCITY = 998
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_STATUS = 1030
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_SINK_TEMPS = 1222
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_DSP_BOARD_TEMPS = 1254
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MC_STATUS = 1286
SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_PRECHARGE_COMPLETED = 454
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1 = 7
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG2 = 39
SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG3 = 71
SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT = 584
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_UV_CUTOFF_NOTIFICATION = 1449
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_CURRENT_MEASUREMENT = 1737
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_PD_FAULT = 1993
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_HORN_AND_LIGHTS = 1481
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_LIGHTS_SYNC = 745
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_POWER_INFO = 105
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_PD_STATUS = 41
SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_BMS_RELAYS = 329
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_STATUS = 42
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_AUX_MEASUREMENTS = 74
SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_DCDC_MEASUREMENTS = 106
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_1 = 683
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_2 = 715
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_3 = 747
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_4 = 779
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_5 = 811
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_6 = 651
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_CURRENT_SENSE = 843
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_STATUS = 875
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_TEMPS = 907
SYSTEM_CAN_MESSAGE_SOLAR_SENSE_SOLAR_INFO = 939
SYSTEM_CAN_MESSAGE_STEERING_STEERING_INFO = 684
SYSTEM_CAN_MESSAGE_TELEMETRY_TEST_MSG = 13
SYSTEM_CAN_MESSAGE_UV_CUTOFF_UV_CUTOFF_NOTIFICATION1 = 1454
# Send single message
def send_message(id, data):
    # id: int -> hex, data: str
    cmd = f"cansend vcan0 {hex(id)[2:].zfill(3)}#{data}"
    subprocess.run(cmd, shell=True)

def pack(num, size):
    if isinstance(num, float) and (size == 32):
        return struct.pack("f", num).hex()
    elif (size == 32):
        return struct.pack("i", num).hex()
    elif (size == 16):
        return struct.pack("h", num).hex()
    elif (size == 8):
        return struct.pack("b", num).hex()
def send_babydriver_babydriver(id, data0, data1, data2, data3, data4, data5, data6):
    send_message(SYSTEM_CAN_MESSAGE_BABYDRIVER_BABYDRIVER,
        pack(id, 8) + 
        pack(data0, 8) + 
        pack(data1, 8) + 
        pack(data2, 8) + 
        pack(data3, 8) + 
        pack(data4, 8) + 
        pack(data5, 8) + 
        pack(data6, 8))

def send_bms_carrier_battery_status(batt_perc, status, fault):
    send_message(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS,
        pack(batt_perc, 16) + 
        pack(status, 8) + 
        pack(fault, 8))

def send_bms_carrier_battery_vt(module_id, voltage, temperature):
    send_message(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT,
        pack(module_id, 16) + 
        pack(voltage, 16) + 
        pack(temperature, 16))

def send_bms_carrier_battery_aggregate_vc(voltage, current):
    send_message(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_AGGREGATE_VC,
        pack(voltage, 32) + 
        pack(current, 32))

def send_bms_carrier_battery_fan_state(fan_1, fan_2, fan_3, fan_4, fan_5, fan_6, fan_7, fan_8):
    send_message(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_FAN_STATE,
        pack(fan_1, 8) + 
        pack(fan_2, 8) + 
        pack(fan_3, 8) + 
        pack(fan_4, 8) + 
        pack(fan_5, 8) + 
        pack(fan_6, 8) + 
        pack(fan_7, 8) + 
        pack(fan_8, 8))

def send_bms_carrier_battery_relay_info(state):
    send_message(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_INFO,
        pack(state, 8))

def send_can_communication_one_shot_msg(sig1, sig2):
    send_message(SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_ONE_SHOT_MSG,
        pack(sig1, 16) + 
        pack(sig2, 16))

def send_can_debug_test_debug(operation, operandA, operandB, operandC):
    send_message(SYSTEM_CAN_MESSAGE_CAN_DEBUG_TEST_DEBUG,
        pack(operation, 8) + 
        pack(operandA, 8) + 
        pack(operandB, 8) + 
        pack(operandC, 8))

def send_centre_console_cc_power_control(power_event, hazard_enabled):
    send_message(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_POWER_CONTROL,
        pack(power_event, 8) + 
        pack(hazard_enabled, 8))

def send_centre_console_drive_output(target_velocity, drive_state, cruise_control, regen_braking, precharge):
    send_message(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_DRIVE_OUTPUT,
        pack(target_velocity, 32) + 
        pack(drive_state, 8) + 
        pack(cruise_control, 8) + 
        pack(regen_braking, 8) + 
        pack(precharge, 8))

def send_charger_request_to_charge(signal1):
    send_message(SYSTEM_CAN_MESSAGE_CHARGER_REQUEST_TO_CHARGE,
        pack(signal1, 8))

def send_charger_charger_connected_state(is_connected):
    send_message(SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_CONNECTED_STATE,
        pack(is_connected, 8))

def send_charger_charger_fault(fault):
    send_message(SYSTEM_CAN_MESSAGE_CHARGER_CHARGER_FAULT,
        pack(fault, 8))

def send_motor_controller_motor_controller_vc(mc_voltage_l, mc_current_l, mc_voltage_r, mc_current_r):
    send_message(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_CONTROLLER_VC,
        pack(mc_voltage_l, 16) + 
        pack(mc_current_l, 16) + 
        pack(mc_voltage_r, 16) + 
        pack(mc_current_r, 16))

def send_motor_controller_motor_velocity(velocity_l, velocity_r):
    send_message(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_VELOCITY,
        pack(velocity_l, 16) + 
        pack(velocity_r, 16))

def send_motor_controller_motor_status(motor_status_l, motor_status_r):
    send_message(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_STATUS,
        pack(motor_status_l, 32) + 
        pack(motor_status_r, 32))

def send_motor_controller_motor_sink_temps(motor_temp_l, heatsink_temp_l, motor_temp_r, heatsink_temp_r):
    send_message(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_SINK_TEMPS,
        pack(motor_temp_l, 16) + 
        pack(heatsink_temp_l, 16) + 
        pack(motor_temp_r, 16) + 
        pack(heatsink_temp_r, 16))

def send_motor_controller_dsp_board_temps(dsp_temp_l, dsp_temp_r):
    send_message(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_DSP_BOARD_TEMPS,
        pack(dsp_temp_l, 32) + 
        pack(dsp_temp_r, 32))

def send_motor_controller_mc_status(limit_bitset_l, error_bitset_l, limit_bitset_r, error_bitset_r, board_fault_bitset, overtemp_bitset, precharge_status):
    send_message(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MC_STATUS,
        pack(limit_bitset_l, 8) + 
        pack(error_bitset_l, 8) + 
        pack(limit_bitset_r, 8) + 
        pack(error_bitset_r, 8) + 
        pack(board_fault_bitset, 8) + 
        pack(overtemp_bitset, 8) + 
        pack(precharge_status, 8))

def send_motor_controller_precharge_completed(notification):
    send_message(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_PRECHARGE_COMPLETED,
        pack(notification, 8))

def send_new_can_transmit_msg1(status):
    send_message(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1,
        pack(status, 8))

def send_new_can_transmit_msg2(signal, signal2):
    send_message(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG2,
        pack(signal, 8) + 
        pack(signal2, 8))

def send_new_can_transmit_msg3(help):
    send_message(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG3,
        pack(help, 8))

def send_pedal_pedal_output(throttle_output, brake_output):
    send_message(SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT,
        pack(throttle_output, 32) + 
        pack(brake_output, 32))

def send_power_distribution_uv_cutoff_notification(signal1):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_UV_CUTOFF_NOTIFICATION,
        pack(signal1, 8))

def send_power_distribution_current_measurement(current_id, current):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_CURRENT_MEASUREMENT,
        pack(current_id, 16) + 
        pack(current, 16))

def send_power_distribution_pd_fault(fault_data, faulting_output):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_PD_FAULT,
        pack(fault_data, 16) + 
        pack(faulting_output, 16))

def send_power_distribution_horn_and_lights(horn_state, lights_state):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_HORN_AND_LIGHTS,
        pack(horn_state, 8) + 
        pack(lights_state, 8))

def send_power_distribution_lights_sync(signal1):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_LIGHTS_SYNC,
        pack(signal1, 8))

def send_power_distribution_power_info(power_state, pd_fault):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_POWER_INFO,
        pack(power_state, 8) + 
        pack(pd_fault, 8))

def send_power_distribution_pd_status(fault_bitset):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_PD_STATUS,
        pack(fault_bitset, 8))

def send_power_distribution_bms_relays(relays_state):
    send_message(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_BMS_RELAYS,
        pack(relays_state, 8))

def send_power_select_power_select_status(status, fault):
    send_message(SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_STATUS,
        pack(status, 8) + 
        pack(fault, 8))

def send_power_select_power_select_aux_measurements(aux_voltage, aux_current, aux_temp, power_supply_current):
    send_message(SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_AUX_MEASUREMENTS,
        pack(aux_voltage, 16) + 
        pack(aux_current, 16) + 
        pack(aux_temp, 16) + 
        pack(power_supply_current, 16))

def send_power_select_power_select_dcdc_measurements(dcdc_voltage, dcdc_current, dcdc_temp, power_supply_voltage):
    send_message(SYSTEM_CAN_MESSAGE_POWER_SELECT_POWER_SELECT_DCDC_MEASUREMENTS,
        pack(dcdc_voltage, 16) + 
        pack(dcdc_current, 16) + 
        pack(dcdc_temp, 16) + 
        pack(power_supply_voltage, 16))

def send_solar_sense_mppt_1(current, voltage, pwm, status):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_1,
        pack(current, 16) + 
        pack(voltage, 16) + 
        pack(pwm, 16) + 
        pack(status, 16))

def send_solar_sense_mppt_2(current, voltage, pwm, status):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_2,
        pack(current, 16) + 
        pack(voltage, 16) + 
        pack(pwm, 16) + 
        pack(status, 16))

def send_solar_sense_mppt_3(current, voltage, pwm, status):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_3,
        pack(current, 16) + 
        pack(voltage, 16) + 
        pack(pwm, 16) + 
        pack(status, 16))

def send_solar_sense_mppt_4(current, voltage, pwm, status):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_4,
        pack(current, 16) + 
        pack(voltage, 16) + 
        pack(pwm, 16) + 
        pack(status, 16))

def send_solar_sense_mppt_5(current, voltage, pwm, status):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_5,
        pack(current, 16) + 
        pack(voltage, 16) + 
        pack(pwm, 16) + 
        pack(status, 16))

def send_solar_sense_mppt_6(current, voltage, pwm, status):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_MPPT_6,
        pack(current, 16) + 
        pack(voltage, 16) + 
        pack(pwm, 16) + 
        pack(status, 16))

def send_solar_sense_current_sense(current, voltage, relay_status):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_CURRENT_SENSE,
        pack(current, 16) + 
        pack(voltage, 16) + 
        pack(relay_status, 8))

def send_solar_sense_thermal_status(overtemp, fullspeed, fan_fail, temp_1, temp_2):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_STATUS,
        pack(overtemp, 8) + 
        pack(fullspeed, 8) + 
        pack(fan_fail, 8) + 
        pack(temp_1, 16) + 
        pack(temp_2, 16))

def send_solar_sense_thermal_temps(temp_3, temp_4, temp_5, temp_6):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_THERMAL_TEMPS,
        pack(temp_3, 16) + 
        pack(temp_4, 16) + 
        pack(temp_5, 16) + 
        pack(temp_6, 16))

def send_solar_sense_solar_info(solar_fault):
    send_message(SYSTEM_CAN_MESSAGE_SOLAR_SENSE_SOLAR_INFO,
        pack(solar_fault, 8))

def send_steering_steering_info(input_cc, input_lights):
    send_message(SYSTEM_CAN_MESSAGE_STEERING_STEERING_INFO,
        pack(input_cc, 8) + 
        pack(input_lights, 8))

def send_telemetry_test_msg(test):
    send_message(SYSTEM_CAN_MESSAGE_TELEMETRY_TEST_MSG,
        pack(test, 8))

def send_uv_cutoff_uv_cutoff_notification1(signal1):
    send_message(SYSTEM_CAN_MESSAGE_UV_CUTOFF_UV_CUTOFF_NOTIFICATION1,
        pack(signal1, 8))


def repeat(repeat_period, send_device_message, *args):
    def threading_repeat(kill_process):
        while not kill_process.is_set():
            send_device_message(*args)
            time.sleep(repeat_period)

    kill_process = threading.Event()

    process_name = threading.Thread(target=threading_repeat, args=(kill_process,))
    process_name.start()

    return process_name, kill_process

def end_repeat(send_device_message_process, kill_process):
    kill_process.set()
    send_device_message_process.join()