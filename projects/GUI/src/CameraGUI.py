#pylint: skip-file
import tkinter as tk
import can
import cantools
import cv2
import PIL
import threading
from PIL import ImageTk
from PIL import Image

'''
sudo modprobe vcan
sudo ip link ad dev vcan0 type vcan
sudo ip link set up vcan0
sudo python3 RPI/GUI.py

v4l2-ctl --list-devices
'''

# change the file path to (can/tools/system_can.dbc)
db = cantools.database.load_file('system_can.dbc')
can_bus = can.interface.Bus(channel='vcan0', bustype='socketcan')
root = tk.Tk()
root.resizable(False, False)
root.geometry('800x480')
root.title("Dashboard")
root.configure(background='black')

vid = cv2.VideoCapture(0)

vid.set(cv2.CAP_PROP_FRAME_WIDTH, 400)
vid.set(cv2.CAP_PROP_FRAME_HEIGHT, 400)

BorderThickness = 2
List = [0, 40, 80, 120, 160, 200, 240, 320, 360, 400]
can_message_list = []
red = '#FF0000'
orange = '#FF9900'
yellow = '#FCFF77'
redlistCount = 0
listCount = 0

GUICount = 0

camera_widget = tk.Label(root, bg='#000000')
camera_widget.place(x=25, y=250)


def open_camera():
    _, frame = vid.read()

    opencv_image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGBA)

    captured_image = Image.fromarray(opencv_image)

    photo_image = ImageTk.PhotoImage(image=captured_image)

    camera_widget.photo_image = photo_image

    camera_widget.configure(image=photo_image)

    camera_widget.after(10, open_camera)


for y in range(3):
    for x in range(3):
        Box = tk.Canvas(root,
                        width=137,
                        height=50,
                        bg='#000000',
                        highlightbackground='white',
                        highlightthickness=BorderThickness).place(x=34 + (x * 137),
                                                                  y=40 + (y * 50))

tk.Label(Box, text='AFE1', font='Montserrat 15', bg='#000000', fg='white').place(x=75, y=6)
tk.Label(Box, text='AFE2', font='Montserrat 15', bg='#000000', fg='white').place(x=209, y=6)
tk.Label(Box, text='AFE3', font='Montserrat 15', bg='#000000', fg='white').place(x=342, y=6)

for y in range(3):
    Box = tk.Canvas(root, width=34, height=50, bg='#000000', highlightbackground='white',
                    highlightthickness=BorderThickness).place(x=0, y=40 + (y * 50))

tk.Label(Box, text='MAX\nTEMP', font='Montserrat 8', bg='#000000', fg='white').place(x=3, y=52)
tk.Label(Box, text='MAX\nVOLT', font='Montserrat 8', bg='#000000', fg='white').place(x=3, y=106)
tk.Label(Box, text='MIN\nVOLT', font='Montserrat 8', bg='#000000', fg='white').place(x=3, y=153)

for x in range(2):
    Box = tk.Canvas(root, width=223, height=50, bg='#000000', highlightbackground='white',
                    highlightthickness=BorderThickness).place(x=0 + (x * 223), y=190)
tk.Label(Box, text='CURRENT', font='Montserrat 10', bg='#000000', fg='white').place(x=10, y=210)
tk.Label(Box, text='VOLT.', font='Montserrat 10', bg='#000000', fg='white').place(x=241, y=210)


def redBox(message, num):
    global redlistCount
    if (listCount > 2):
        listCount = 0
    RedBox = tk.Canvas(
        root,
        width=357,
        height=40,
        bg=red,
        highlightthickness=0).place(
        x=448,
        y=List[redlistCount])
    tk.Label(RedBox, text=message, font='Montserrat 10', bg=red,
             fg='black').place(x=450, y=(List[redlistCount]))
    tk.Label(RedBox, text=num, font='Montserrat 10', bg=red,
             fg='black').place(x=743, y=(List[redlistCount]))
    redlistCount += 1


def orangeBox(message, num):
    global listCount
    if (listCount > 7):
        listCount = 0
    OrangeBox = tk.Canvas(root, width=357, height=40, bg=orange,
                          highlightthickness=0).place(x=448, y=List[listCount + 3])
    tk.Label(OrangeBox, text=message, font='Montserrat 10', bg=orange,
             fg='black').place(x=450, y=(List[listCount + 3]))
    tk.Label(OrangeBox, text=num, font='Montserrat 10', bg=orange,
             fg='black').place(x=743, y=(List[listCount + 3]))
    listCount += 1


def yellowBox(message, num):
    global listCount
    if (listCount > 7):
        listCount = 0
    YellowBox = tk.Canvas(root, width=357, height=40, bg=yellow,
                          highlightthickness=0).place(x=448, y=List[listCount + 3])
    tk.Label(YellowBox, text=message, font='Montserrat 10', bg=yellow,
             fg='black').place(x=450, y=(List[listCount + 3]))
    tk.Label(YellowBox, text=num, font='Montserrat 10', bg=yellow,
             fg='black').place(x=743, y=(List[listCount + 3]))
    listCount += 1
    # EmptyBox = tk.Canvas(root, width=445, height=39, bg='black', highlightthickness=0).place(x=355, y=List[0])


def nothing(message):
    pass


def display_pd_status(bitset):
    if (bitset == 1):
        orangeBox('Aux Disconnect', 0)
    if (bitset == 2):
        orangeBox('DCDC Disconnect', 0)
    if (bitset == 3):
        orangeBox('Aux Disconnect', 0)
        orangeBox('DCDC Disconnect', 0)


def BMS_Fault(message):
    global can_message_list

    print(message)
    faults = {
        'BMS_FAULT_OVERVOLTAGE': 0,
        'BMS_FAULT_UNBALANCE': 1,
        'BMS_FAULT_OVERTEMP_AMBIENT': 2,
        'BMS_FAULT_COMMS_LOSS_AFE': 3,
        'BMS_FAULT_COMMS_LOSS_CURR_SENSE': 4,
        'BMS_FAULT_OVERTEMP_CELL': 5,
        'BMS_FAULT_OVERCURRENT': 6,
        'BMS_FAULT_UNDERVOLTAGE': 7,
        'BMS_FAULT_KILLSWITCH': 8,
        'BMS_FAULT_RELAY_CLOSE_FAILED': 9
    }
    fault_bits = message['fault']
    for fault_name, bit_position in faults.items():
        if fault_bits & (1 << bit_position):
            if (fault_name not in can_message_list):
                can_message_list.append(fault_name)
                if (fault_name == 'BMS_FAULT_OVERVOLTAGE'):
                    redBox(fault_name, 0)
                if (fault_name == 'BMS_FAULT_UNBALANCE'):
                    redBox(fault_name, 0)
                if (fault_name == 'BMS_FAULT_OVERTEMP_AMBIENT'):
                    redBox(fault_name, 0)
                else:
                    orangeBox(fault_name, 0)


def mc_error_bitset_l(bitset):
    if (bitset == 1):
        orangeBox("Software overcurrent(L)", 0)
    if (bitset == 2):
        orangeBox("DC Bus Overvoltage(L)", 0)
    if (bitset == 3):
        orangeBox("Bad Motor Position HAL sequence(L)", 0)
    if (bitset == 4):
        orangeBox("Watchdog caused last reset(L)", 0)
    if (bitset == 5):
        orangeBox("Config read errors(L)", 0)
    if (bitset == 6):
        orangeBox("15V rail under voltage lockout(L)", 0)
    if (bitset == 7):
        orangeBox("Desaturation fault(L)", 0)
    if (bitset == 8):
        orangeBox("Motor Overspeed(L)", 0)


def mc_error_bitset_r(bitset):
    if (bitset == 1):
        orangeBox("Software overcurrent(R)", 0)
    if (bitset == 2):
        orangeBox("DC Bus Overvoltage(R)", 0)
    if (bitset == 3):
        orangeBox("Bad Motor Position HAL sequence(R)", 0)
    if (bitset == 4):
        orangeBox("Watchdog caused last reset(R)", 0)
    if (bitset == 5):
        orangeBox("Config read errors(R)", 0)
    if (bitset == 6):
        orangeBox("15V rail under voltage lockout(R)", 0)
    if (bitset == 7):
        orangeBox("Desaturation fault(R)", 0)
    if (bitset == 8):
        orangeBox("Motor Overspeed(R)", 0)


DISPLAY_MSG_DICT = {
    db.get_message_by_name('battery_vt').frame_id: [(nothing, 'voltage'), (nothing, 'current'), (nothing, 'temperature'), (nothing, 'batt_perc')],
    db.get_message_by_name('battery_status').frame_id: [(BMS_Fault, 'fault'), (nothing, 'aux_batt_v')],
    # something for fault
    db.get_message_by_name('pd_status').frame_id: [(nothing, 'power_state'), (display_pd_status, 'fault_bitset'), (nothing, 'bps_persist')],
    db.get_message_by_name('mc_status').frame_id: [(nothing, 'limit_bitset_l'), (mc_error_bitset_l, 'error_bitset_l'), (nothing, 'limit_bitset_l'),
                                                   (mc_error_bitset_r, 'error_bitset_r'), (nothing,
                                                                                           'board_fault_bitset'),
                                                   (nothing, 'precharge_status')]
}

DISPLAY_MSG_DICT_CURRENT_SENSE = {
    db.get_message_by_name('battery_vt').frame_id
}

DISPLAY_MSG_AFES = {
    db.get_message_by_name('AFE1_status').frame_id,
    db.get_message_by_name('AFE2_status').frame_id,
    db.get_message_by_name('AFE3_status').frame_id,
}

AFE_Data = [{
    "Temp": [None, None, None],
    "Voltages":[None, None, None, None, None, None, None, None, None, None, None, None]
},

    {
    "Temp": [None, None, None],
    "Voltages":[None, None, None, None, None, None, None, None, None, None, None, None]
},

    {
    "Temp": [None, None, None],
    "Voltages":[None, None, None, None, None, None, None, None, None, None, None, None]
}]


def AFE1FullUpdate():
    global AFE1MaxTemp, AFE1Max, AFE1Min

    if None not in AFE_Data[0]["Temp"]:
        AFE1MaxTemp = max(AFE_Data[0]["Temp"])
        AFE_Data[0]["Temp"] = [None] * 3

    if None not in AFE_Data[0]["Voltages"]:
        AFE1Max = max(AFE_Data[0]["Voltage"])
        AFE1Min = min(AFE_Data[0]["Voltage"])

        tk.Label(
            Box,
            text=AFE1MaxTemp,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=47,
            y=60)
        tk.Label(
            Box,
            text=AFE1Max,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=47,
            y=114)
        tk.Label(
            Box,
            text=AFE1Min,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=47,
            y=161)

        AFE_Data[0]["Voltages"] = [None] * 12


def AFE2FullUpdate():
    global AFE2MaxTemp, AFE2Max, AFE2Min

    if None not in AFE_Data[1]["Temp"]:
        AFE2MaxTemp = max(AFE_Data[1]["Temp"])
        AFE_Data[1]["Temp"] = [None] * 3

    if None not in AFE_Data[1]["Voltages"]:
        AFE2Max = max(AFE_Data[1]["Voltage"])
        AFE2Min = min(AFE_Data[1]["Voltage"])

        tk.Label(
            Box,
            text=AFE2MaxTemp,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=184,
            y=60)
        tk.Label(
            Box,
            text=AFE2Max,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=184,
            y=114)
        tk.Label(
            Box,
            text=AFE2Min,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=184,
            y=161)

        AFE_Data[1]["Voltages"] = [None] * 12


def AFE3FullUpdate():
    global AFE3MaxTemp, AFE3Max, AFE3Min

    if None not in AFE_Data[2]["Temp"]:
        AFE3MaxTemp = max(AFE_Data[2]["Temp"])
        AFE_Data[2]["Temp"] = [None] * 3

    if None not in AFE_Data[2]["Voltages"]:
        AFE3Max = max(AFE_Data[2]["Voltage"])
        AFE3Min = min(AFE_Data[2]["Voltage"])

        tk.Label(
            Box,
            text=AFE3MaxTemp,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=323,
            y=60)
        tk.Label(
            Box,
            text=AFE3Max,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=323,
            y=114)
        tk.Label(
            Box,
            text=AFE3Min,
            font='Montserrat 8',
            bg='#000000',
            fg='white').place(
            x=323,
            y=161)

        AFE_Data[2]["Voltages"] = [None] * 12


def handle_message(msg):
    global AFE1Max, AFE1Min, AFE2Max, AFE2Min, AFE3Max, AFE3Min, GUICount, redlistCount, listCount

    GUICount += 1
    if (GUICount % 10 == 0):
        can_message_list.clear()
        for x in range(10):
            BlackBox = tk.Canvas(
                root,
                width=357,
                height=40,
                bg='#000000',
                highlightthickness=0).place(
                x=448,
                y=List[x])

    try:
        combinedSpeed = 0
        decoded_message = db.decode_message(msg.arbitration_id, msg.data)
        # print(decoded_message)
        if msg.arbitration_id in DISPLAY_MSG_DICT.keys():
            for sig in DISPLAY_MSG_DICT[msg.arbitration_id]:
                if(sig[1] == 'fault'):
                    sig[0](decoded_message)
                else:
                    sig[0](decoded_message[sig[1]])

        if msg.arbitration_id in DISPLAY_MSG_AFES:
            afe_index = DISPLAY_MSG_AFES.index(msg.arbitration_id)
            if decoded_message["id"] < 3:  # only need 3 temperatures
                # which AFE, temp/voltage, index in that array
                AFE_Data[afe_index]["Temp"][decoded_message["id"]] = decoded_message["temp"]
            AFE_Data[afe_index]["Voltages"][3 * decoded_message["id"]] = decoded_message["v1"] / 1000
            AFE_Data[afe_index]["Voltages"][3 *
                                            decoded_message["id"] + 1] = decoded_message["v2"] / 1000
            AFE_Data[afe_index]["Voltages"][3 *
                                            decoded_message["id"] + 2] = decoded_message["v3"] / 1000
            AFE1FullUpdate()
            AFE2FullUpdate()
            AFE3FullUpdate()

        if msg.arbitration_id in DISPLAY_MSG_DICT_CURRENT_SENSE:
            tk.Label(
                Box,
                text=(
                    (decoded_message['current'] / 1000),
                    'A'),
                font='Montserrat 16',
                width=6,
                bg='#000000',
                fg='white').place(
                x=438,
                y=18)
            print(decoded_message['current'])
            tk.Label(
                Box,
                text=(
                    (decoded_message['voltage'] / 1000),
                    'A'),
                font='Montserrat 16',
                width=6,
                bg='#000000',
                fg='white').place(
                x=709,
                y=18)
            print(decoded_message['voltage'])

    except KeyError:
        print("Unknown message:", msg)
# update_displays()


def camera_thread():
    open_camera()


t1 = threading.Thread(target=camera_thread)
t1.start()

notifier = can.Notifier(can_bus, [handle_message])

root.mainloop()
