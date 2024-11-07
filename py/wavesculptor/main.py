'''
Simulate a wavesculptor 22's can messages
'''
import subprocess
import time
import struct
from collections import defaultdict
import threading
from can_a.util import set_output, parse_line

DRIVER_CONTROL_BASE = 0x00
MOTOR_CONTROLLER_BASE_L = 0x40
MOTOR_CONTROLLER_BASE_R = 0x80
CAN = "can0"


def pack(num, size):
    '''pack a variable into the given size in hex'''
    if isinstance(num, float) and (size == 32):
        return struct.pack("f", num).hex()
    if size == 32:
        return struct.pack("i", num).hex()
    if size == 16:
        return struct.pack("h", num).hex()
    if size == 8:
        return struct.pack("b", num).hex()
    return ""


def send_message(can_id, d1, d2):
    '''send a message with two floats'''
    data = pack(d1, 32) + pack(d2, 32)
    subprocess.run(["cansend", CAN, f"{can_id:0>3x}#{data}"], check=False)


def send_status(can_id, data):
    '''send the status message for wavesculptor'''
    subprocess.run(["cansend", CAN, f"{can_id:0>3x}#{data}"], check=False)


def pack_status(rx_err, tx_err, active, err_flg, lim_flg):
    '''pack the status into a hex string'''
    return pack(rx_err, 8) + pack(tx_err, 8) + pack(active, 16) + \
        pack(err_flg, 16) + pack(lim_flg, 16)


def handle_stdin(state):
    '''handle stdin commands'''


def send_200ms_messages(current, velocity):
    '''send can messages that are sent every 200ms'''
    for i, base_addrs in enumerate([MOTOR_CONTROLLER_BASE_L, MOTOR_CONTROLLER_BASE_R]):
        send_status(base_addrs + 0x01,
                    pack_status(0, 0, 0, 0, 0))  # status
        send_message(base_addrs + 0x02, current + i,
                     current * 2 + i)  # bus current, bus voltage
        send_message(base_addrs + 0x03, velocity + i,
                     velocity * 2 + i)  # vehicle vel, motor rpm

        # these are all ignored so values doens't matter,
        # just make sure they don't break anything
        # phase B cur, phase C cur
        send_message(base_addrs + 0x04, 0.2, 0.2)
        send_message(base_addrs + 0x05, 0.2, 0.2)  # vol vec Vd, Vq
        send_message(base_addrs + 0x06, 0.2, 0.2)  # cur vec Id, Iq
        send_message(base_addrs + 0x07, 0.1, 0.1)  # backEmf d, q


def send_1s_messages(offset=0):
    '''send can messages that are sent every 1s'''
    for i, base_addrs in enumerate([MOTOR_CONTROLLER_BASE_L, MOTOR_CONTROLLER_BASE_R]):
        send_message(base_addrs + 0x00, 0, 0)  # id
        send_message(base_addrs + 0x08, 15.0 + offset + i, 0)  # 15V rail V
        send_message(base_addrs + 0x09, 3.3 + offset + i,
                     1.9 + offset + i)  # 3.3 rail V, 1.9 rail V
        send_message(base_addrs + 0x0b, 30.0 + offset + i,
                     31.0 + offset + i)  # heatsink temp, motor temp
        send_message(base_addrs + 0x0c, 0,
                     25.0 + offset + i)  # ---,  dsp temp
        send_message(base_addrs + 0x0e, 45.0 + offset + i,
                     15.0 + offset + i)  # DC bus amp hours, odometer


def update_display(motor_velocity, rx):
    '''update terminal output'''
    set_output(f'''
WAVESCULPTOR:
motor_vel: {motor_velocity}
current:   {rx["current"]}
velocity:  {rx["velocity"]}
''')


def handle_rx(rx):
    '''handle rx from can device'''
    with subprocess.Popen(['candump', CAN], stdout=subprocess.PIPE) as proc:
        while True:
            line = proc.stdout.readline().decode().strip()
            can_id, data = parse_line(line)

            if can_id == DRIVER_CONTROL_BASE + 1:
                rx["current"] = struct.unpack("<f", data[0:4])[0]
                rx["velocity"] = struct.unpack("<f", data[4:8])[0]


def main():
    '''main'''
    wake_time = time.time()
    counter = 0
    counter_2 = 0

    motor_velocity = 0

    state = defaultdict(int)
    rx = defaultdict(int)

    stdin_thread = threading.Thread(target=handle_stdin, args=(state,))
    stdin_thread.start()

    rx_thread = threading.Thread(target=handle_rx, args=(rx,))
    rx_thread.start()

    while True:
        motor_velocity = motor_velocity * 0.95
        if motor_velocity < rx["velocity"]:
            motor_velocity += min(rx["current"],
                                  rx["velocity"] - motor_velocity)
        if motor_velocity > rx["velocity"]:
            motor_velocity -= min(rx["current"],
                                  motor_velocity - rx["velocity"])

        # every 200 ms
        send_200ms_messages(rx["current"], motor_velocity)
        update_display(motor_velocity, rx)

        counter += 1
        if counter % 5:
            # every 1 second
            counter_2 += 1
            if counter_2 % 10:
                counter_2 = 0

            send_1s_messages(counter_2 * 0.1)

            counter = 0

        wake_time += 0.2
        time.sleep(max(wake_time - time.time(), 0))


if __name__ == "__main__":
    main()
