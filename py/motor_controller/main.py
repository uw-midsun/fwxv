'''
Testing motor controller interface
'''
import subprocess
import os
import time
import struct
import sys
from collections import defaultdict
from can import can
from can.util import set_output, parse_line

DRIVE_STATE_STR = ["NEUTRAL", "DRIVE", "REVERSE"]


def handle_stdin():
    '''handle stdin input, updates velocity, drive state, etc.'''
    state = {}
    line = sys.stdin.readline().strip()
    if line:
        # output_str = ""
        try:
            cmd, val = line.split(" ")
            if cmd == "vel":
                state["vel"] = float(val)
            elif cmd == "state":
                state["state"] = DRIVE_STATE_STR.index(val.upper())
            elif cmd == "throttle":
                state["throttle"] = float(val)
            elif cmd == "brake":
                state["brake"] = float(val)
        except BaseException:
            pass
    return state


def update_display(state, rx):
    '''update terminal output'''
    set_output(f"""
state: {DRIVE_STATE_STR[state['state']]}, vel: {state['vel']:>5}, throttle: {state['throttle']}, brake: {state['brake']}
            left      right
velocity:    {rx["vel_l"]:<8d}  {rx["vel_r"]:<8d}
voltage:     {rx["vol_l"]:<8d}  {rx["vol_r"]:<8d}
current:     {rx["cur_l"]:<8d}  {rx["cur_r"]:<8d}
motor temp:  {rx["mtt_l"]:<8d}  {rx["mtt_r"]:<8d}
sink temp:   {rx["hst_l"]:<8d}  {rx["hst_r"]:<8d}
dsp temp:    {rx["dsp_l"]:<8d}  {rx["dsp_r"]:<8d}
motor stat:  {rx["mts_l"]:<8d}  {rx["mts_r"]:<8d}
limit:       {rx["lbs_l"]:0<8b}  {rx["lbs_r"]:0<8b}
error:       {rx["ebs_l"]:0<8b}  {rx["ebs_r"]:0<8b}
board fault: {rx["bfalt"]}   overtemp: {rx["ovtmp"]:}    precharge: {rx["prchg"]}
""")


def rx_all(proc):
    '''rx all msgs from can'''
    rx = {}
    line = proc.stdout.readline().decode().strip()
    while line:
        can_id, data = parse_line(line)

        if can_id == can.SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_CONTROLLER_VC:
            rx["vol_l"] = struct.unpack("h", data[0:2])[0]
            rx["cur_l"] = struct.unpack("h", data[2:4])[0]
            rx["vol_r"] = struct.unpack("h", data[4:6])[0]
            rx["cur_r"] = struct.unpack("h", data[6:8])[0]
        elif can_id == can.SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_VELOCITY:
            rx["vel_l"] = struct.unpack("h", data[0:2])[0]
            rx["vel_r"] = struct.unpack("h", data[2:4])[0]
        elif can_id == can.SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_STATUS:
            rx["mts_l"] = struct.unpack("i", data[0:4])[0]
            rx["mts_r"] = struct.unpack("i", data[4:8])[0]
        elif can_id == can.SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MOTOR_SINK_TEMPS:
            rx["mtt_l"] = struct.unpack("h", data[0:2])[0]
            rx["hst_l"] = struct.unpack("h", data[2:4])[0]
            rx["mtt_r"] = struct.unpack("h", data[4:6])[0]
            rx["hst_r"] = struct.unpack("h", data[6:8])[0]
        elif can_id == can.SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_DSP_BOARD_TEMPS:
            rx["dsp_l"] = struct.unpack("i", data[0:4])[0]
            rx["dsp_r"] = struct.unpack("i", data[4:8])[0]
        elif can_id == can.SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_MC_STATUS:
            rx["lbs_l"] = int(data[0])
            rx["ebs_l"] = int(data[1])
            rx["lbs_r"] = int(data[2])
            rx["ebs_r"] = int(data[3])
            rx["bfalt"] = int(data[4])
            rx["ovtmp"] = int(data[5])
            rx["prchg"] = int(data[6])

        line = proc.stdout.readline().decode().strip()
    return rx


def main():
    '''main'''
    with subprocess.Popen(['candump', 'vcan0'], stdout=subprocess.PIPE) as proc:
        os.set_blocking(proc.stdout.fileno(), False)
        os.set_blocking(sys.stdin.fileno(), False)
        wake_time = time.time()

        state = {
            "vel": 0,
            "state": 0,
            "throttle": 0,
            "brake": 0,
        }

        rx = defaultdict(int)

        while True:
            # std input
            state |= handle_stdin()
            # Can RX
            rx |= rx_all(proc)

            update_display(state, rx)

            # every 200 ms
            can.send_centre_console_drive_output(
                state["vel"], state["state"], 1, 1, 1)
            can.send_pedal_pedal_output(state["throttle"], state["brake"])

            wake_time += 0.2
            time.sleep(max(wake_time - time.time(), 0))


if __name__ == "__main__":
    main()
