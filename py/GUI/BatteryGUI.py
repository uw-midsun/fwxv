# pylint: skip-file
import tkinter as tk
import can
import cantools
from multiprocessing.connection import Client
import subprocess
import sys
import time

"""
For VCAN:
sudo modprobe vcan
sudo ip link ad dev vcan0 type vcan
sudo ip link set up vcan0

For CAN:
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan

sudo ip link set can0 up type can bitrate 500000
sudo ip link set can0 up
"""

# Wait 30 sec after RPi autostart for listener to complete setup 
time.sleep(30)

# Multiprocessing client
address = ('localhost', 6000)
client = Client(address, authkey=b'secret password')

def initialize(command):
    try:
        result = subprocess.run(
            command, shell=True, check=True, text=True, capture_output=True
        )
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error ocurred while running command: {command}")
        print(f"Error: {e}")
        sys.exit(1)


commands = [
    "sudo modprobe can",
    "sudo modprobe can_raw",
    "sudo modprobe vcan",
    "sudo ip link set can0 up type can bitrate 500000",
    "sudo ip link set can0 up",
]

for command in commands:
    output = initialize(command)
    if output:
        print(output)

# change the file path to (can/tools/system_can.dbc)
db = cantools.database.load_file("/home/midnightsun/GUI/system_can.dbc")
can_bus = can.interface.Bus(channel="can0", bustype="socketcan")
root = tk.Tk()
root.resizable(False, False)
root.geometry("800x480")
root.title("Dashboard")
root.configure(background="black")
BorderThickness = 2

red = "#FF0000"
orange = "#FF9900"
yellow = "#FCFF77"
listCount = 0

AFE1Max = None
AFE1Min = None

AFE2Max = None
AFE2Min = None

AFE3Max = None
AFE3Min = None

CurrentBox = tk.Canvas(
    root,
    width=268,
    height=65,
    bg="#000000",
    highlightbackground="white",
    highlightthickness=BorderThickness,
).place(x=265, y=0)
tk.Label(
    CurrentBox, text="CURRENT", font="Montserrat 20", bg="#000000", fg="white"
).place(x=278, y=18)
VoltBox = tk.Canvas(
    root,
    width=268,
    height=65,
    bg="#000000",
    highlightbackground="white",
    highlightthickness=BorderThickness,
).place(x=533, y=0)
tk.Label(VoltBox, text="VOLT.", font="Montserrat 20", bg="#000000", fg="white").place(
    x=549, y=18
)

tk.Label(root, text="AFE1.", font="Montserrat 20", bg="#000000", fg="white").place(
    x=5, y=57
)
tk.Label(root, text="AFE2.", font="Montserrat 20", bg="#000000", fg="white").place(
    x=5, y=191
)
tk.Label(root, text="AFE3.", font="Montserrat 20", bg="#000000", fg="white").place(
    x=5, y=326
)

for y in range(2):  # AFE1
    for x in range(3):
        Box = tk.Canvas(
            root,
            width=133,
            height=51.5,
            bg="#000000",
            highlightbackground="white",
            highlightthickness=BorderThickness,
        ).place(x=133 * (x), y=88 + (51.5 * y))

for y in range(3):
    for x in range(4):
        Box = tk.Canvas(
            root,
            width=93,
            height=35,
            bg="#000000",
            highlightbackground="white",
            highlightthickness=BorderThickness,
        ).place(x=428 + 93 * (x), y=88 + (35 * y))

for y in range(2):  # AFE2
    for x in range(3):
        Box = tk.Canvas(
            root,
            width=133,
            height=51.5,
            bg="#000000",
            highlightbackground="white",
            highlightthickness=BorderThickness,
        ).place(x=133 * (x), y=222 + (51.5 * y))

for y in range(3):
    for x in range(4):
        Box = tk.Canvas(
            root,
            width=93,
            height=35,
            bg="#000000",
            highlightbackground="white",
            highlightthickness=BorderThickness,
        ).place(x=428 + 93 * (x), y=222 + (35 * y))

for y in range(2):  # AFE3
    for x in range(3):
        Box = tk.Canvas(
            root,
            width=133,
            height=51.5,
            bg="#000000",
            highlightbackground="white",
            highlightthickness=BorderThickness,
        ).place(x=133 * (x), y=355 + (51.5 * y))

for y in range(3):
    for x in range(4):
        Box = tk.Canvas(
            root,
            width=93,
            height=35,
            bg="#000000",
            highlightbackground="white",
            highlightthickness=BorderThickness,
        ).place(x=428 + 93 * (x), y=355 + (35 * y))

# AFE1-------------------------------


def updateAFE1(box, num):
    if box < 4:
        tk.Label(
            Box, text=num, font="Montserrat 20", width=6, bg="#000000", fg="white"
        ).place(x=5 + 133 * (box - 1), y=107)
    if box >= 4 and box < 7:
        tk.Label(
            Box, text=num, font="Montserrat 20", width=6, bg="#000000", fg="white"
        ).place(x=5 + 133 * (box - 4), y=158.5)


def update_AFE1_cells(box, num):
    if box < 5:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 1), y=90 + (35 * 0))
    if box >= 5 and box < 9:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 5), y=90 + (35 * 1))
    if box >= 9 and box < 13:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 9), y=90 + (35 * 2))


# AFE2-------------------------------


def updateAFE2(box, num):
    if box < 4:
        tk.Label(
            Box, text=num, font="Montserrat 20", width=6, bg="#000000", fg="white"
        ).place(x=4 + 133 * (box - 1), y=231)
    if box >= 4 and box < 7:
        tk.Label(
            Box, text=num, font="Montserrat 20", width=6, bg="#000000", fg="white"
        ).place(x=4 + 133 * (box - 4), y=282.5)


def update_AFE2_cells(box, num):
    if box < 5:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 1), y=224 + (35 * 0))
    if box >= 5 and box < 9:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 5), y=224 + (35 * 1))
    if box >= 9 and box < 13:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 9), y=224 + (35 * 2))


# AFE3-------------------------------


def updateAFE3(box, num):
    if box < 4:
        tk.Label(
            Box, text=num, font="Montserrat 20", width=6, bg="#000000", fg="white"
        ).place(x=4 + 133 * (box - 1), y=364)
    if box >= 4 and box < 7:
        tk.Label(
            Box, text=num, font="Montserrat 20", width=6, bg="#000000", fg="white"
        ).place(x=4 + 133 * (box - 4), y=415.5)


def update_AFE3_cells(box, num):
    if box < 5:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 1), y=357 + (35 * 0))
    if box >= 5 and box < 9:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 5), y=357 + (35 * 1))
    if box >= 9 and box < 13:
        tk.Label(
            Box, text=num, font="Montserrat 18", width=5, bg="#000000", fg="white"
        ).place(x=436 + 93 * (box - 9), y=357 + (35 * 2))


DISPLAY_MSG_DICT_CURRENT_SENSE = {
    db.get_message_by_name("battery_vt").frame_id}

DISPLAY_MSG_AFES = [
    db.get_message_by_name("AFE1_status").frame_id,
    db.get_message_by_name("AFE2_status").frame_id,
    db.get_message_by_name("AFE3_status").frame_id,
]

"""
AFE1_status {id: 0, temp: 10, v1: 10, v2: 10, v3:10} temp1, v1, v2, v3
AFE2_status {id: 0, temp: 10, v1: 10, v2: 10, v3:10}
AFE3_status {id: 0, temp: 10, v1: 10, v2: 10, v3:10}
AFE1_status {id: 1, temp: 10, v1: 10, v2: 10, v3:10} temp1, v4, v5, v6
AFE2_status {id: 1, temp: 10, v1: 10, v2: 10, v3:10}
AFE3_status {id: 1, temp: 10, v1: 10, v2: 10, v3:10}
AFE1_status {id: 2, temp: 10, v1: 10, v2: 10, v3:10} temp1, v7, v8, v9
AFE2_status {id: 2, temp: 10, v1: 10, v2: 10, v3:10}
AFE3_status {id: 2, temp: 10, v1: 10, v2: 10, v3:10}
AFE1_status {id: 3, temp: 10, v1: 10, v2: 10, v3:10} temp1, v10, v11, v12
AFE2_status {id: 3, temp: 10, v1: 10, v2: 10, v3:10}
AFE3_status {id: 3, temp: 10, v1: 10, v2: 10, v3:10}
"""

AFE_Data = [
    {
        "Temp": [None, None, None],
        "Voltages": [
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
        ],
    },
    {
        "Temp": [None, None, None],
        "Voltages": [
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
        ],
    },
    {
        "Temp": [None, None, None],
        "Voltages": [
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
        ],
    },
]


def AFE1FullUpdate():
    global AFE1Max, AFE1Min

    if None not in AFE_Data[0]["Temp"]:
        updateAFE1(1, (AFE_Data[0]["Temp"][0], "C"))
        updateAFE1(2, (AFE_Data[0]["Temp"][1], "C"))
        updateAFE1(3, (AFE_Data[0]["Temp"][2], "C"))
        AFE_Data[0]["Temp"] = [None] * 3

    if None not in AFE_Data[0]["Voltages"]:
        for x in range(1, 13):
            update_AFE1_cells(x, AFE_Data[0]["Voltages"][x - 1])

        AFE1Max = max(AFE_Data[0]["Voltages"])
        AFE1Min = min(AFE_Data[0]["Voltages"])
        Unbalance = AFE1Max - AFE1Min

        updateAFE1(4, AFE1Max)
        updateAFE1(5, AFE1Min)
        updateAFE1(6, Unbalance)

        AFE_Data[0]["Voltages"] = [None] * 12


def AFE2FullUpdate():
    global AFE2Max, AFE2Min

    if None not in AFE_Data[1]["Temp"]:
        updateAFE2(1, (AFE_Data[1]["Temp"][0], "C"))
        updateAFE2(2, (AFE_Data[1]["Temp"][1], "C"))
        updateAFE2(3, (AFE_Data[1]["Temp"][2], "C"))
        AFE_Data[1]["Temp"] = [None] * 3

    if None not in AFE_Data[1]["Voltages"]:
        for x in range(1, 13):
            update_AFE2_cells(x, AFE_Data[1]["Voltages"][x - 1])

        AFE2Max = max(AFE_Data[1]["Voltages"])
        AFE2Min = min(AFE_Data[1]["Voltages"])
        Unbalance = AFE2Max - AFE2Min

        updateAFE2(4, AFE2Max)
        updateAFE2(5, AFE2Min)
        updateAFE2(6, Unbalance)

        AFE_Data[1]["Voltages"] = [None] * 12


def AFE3FullUpdate():
    global AFE3Max, AFE3Min

    if None not in AFE_Data[2]["Temp"]:
        updateAFE3(1, (AFE_Data[2]["Temp"][0], "C"))
        updateAFE3(2, (AFE_Data[2]["Temp"][1], "C"))
        updateAFE3(3, (AFE_Data[2]["Temp"][2], "C"))
        AFE_Data[2]["Temp"] = [None] * 3

    if None not in AFE_Data[2]["Voltages"]:
        for x in range(1, 13):
            update_AFE3_cells(x, AFE_Data[2]["Voltages"][x - 1])

        AFE3Max = max(AFE_Data[2]["Voltages"])
        AFE3Min = min(AFE_Data[2]["Voltages"])
        Unbalance = AFE3Max - AFE3Min

        updateAFE3(4, AFE3Max)
        updateAFE3(5, AFE3Min)
        updateAFE3(6, Unbalance)

        AFE_Data[2]["Voltages"] = [None] * 12


def handle_message(msg):
    global AFE1Max, AFE1Min, AFE2Max, AFE2Min, AFE3Max, AFE3Min

    try:
        print("Listener Executed")
        decoded_message = db.decode_message(msg.arbitration_id, msg.data)
        client.send((msg.arbitration_id, decoded_message))
        if msg.arbitration_id in DISPLAY_MSG_AFES:
            afe_index = DISPLAY_MSG_AFES.index(
                msg.arbitration_id
            )  # Figures out which AFE
            if decoded_message["id"] < 3:  # only need 3 temperatures
                # which AFE, temp/voltage, index in that array
                AFE_Data[afe_index]["Temp"][decoded_message["id"]] = decoded_message[
                    "temp"
                ]
            AFE_Data[afe_index]["Voltages"][3 * decoded_message["id"]] = round(
                decoded_message["v1"] / 10000, 2
            )
            AFE_Data[afe_index]["Voltages"][3 * decoded_message["id"] + 1] = round(
                decoded_message["v2"] / 10000, 2
            )
            AFE_Data[afe_index]["Voltages"][3 * decoded_message["id"] + 2] = round(
                decoded_message["v3"] / 10000, 2
            )
            AFE1FullUpdate()
            AFE2FullUpdate()
            AFE3FullUpdate()

        if msg.arbitration_id in DISPLAY_MSG_DICT_CURRENT_SENSE:
            tk.Label(
                Box,
                text=((decoded_message["current"] / 1000), "A"),
                font="Montserrat 16",
                width=6,
                bg="#000000",
                fg="white",
            ).place(x=438, y=18)
            print(decoded_message["current"])
            tk.Label(
                Box,
                text=((decoded_message["voltage"] / 1000), "V"),
                font="Montserrat 16",
                width=6,
                bg="#000000",
                fg="white",
            ).place(x=709, y=18)
            print(decoded_message["voltage"])

    except KeyError:
        print("Unknown message:", msg)


# update_displays()


def main():
    notifier = can.Notifier(can_bus, [handle_message])

    while True:
        root.mainloop()


if __name__ == "__main__":
    main()
