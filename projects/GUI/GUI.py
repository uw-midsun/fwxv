import tkinter as tk
import can
import cantools

'''
sudo modprobe vcan
sudo ip link ad dev vcan0 type vcan
sudo ip link set up vcan0
sudo python3 RPI/GUI.py
'''
db = cantools.database.load_file('RPI/can/system_can.dbc') #change the file path to (can/tools/system_can.dbc)
can_bus = can.interface.Bus(channel='vcan0', bustype='socketcan')
root = tk.Tk()
root.resizable(False, False)
root.geometry('800x480')
root.title("Dashboard")
BorderThickness = 5
List = [0,40, 80, 120, 160, 200]
red = '#FF0000'
orange = '#FF9900'
yellow = '#FCFF77'
listCount = 0
BatteryBox = tk.Canvas(root, width=345, height=63, bg='#1b2838', highlightbackground='white', highlightthickness=BorderThickness).place(x=0, y=0)
tk.Label(BatteryBox, text="BATTERY", font='Montserrat 25', bg='#1b2838', fg='white').place(x=20, y=17)
TempBox = tk.Canvas(root, width=345, height=63, bg='#1b2838', highlightbackground='white', highlightthickness=BorderThickness).place(x=0, y=60)
tk.Label(TempBox, text='TEMP', font='Montserrat 25', bg='#1b2838', fg='white').place(x=20, y=77)
SpeedBox = tk.Canvas(root, width=345, height=63, bg='#1b2838', highlightbackground='white', highlightthickness=BorderThickness).place(x=0, y=120)
tk.Label(SpeedBox, text='SPEED', font='Montserrat 25', bg='#1b2838', fg='white').place(x=20, y=134)
TargetSpeedBox = tk.Canvas(root, width=345, height=63, bg='#1b2838', highlightbackground='white', highlightthickness=BorderThickness).place(x=0, y=180)
tk.Label(TargetSpeedBox, text='CC SPEED', font='Montserrat 25', bg='#1b2838', fg='white').place(x=20, y=193)
speedCount = 0

def redBox(message, num):
    global listCount
    if (listCount > 5):
        listCount = 0
    RedBox = tk.Canvas(root, width=455, height=40, bg=red, highlightthickness=0).place(x=355, y=List[listCount])
    tk.Label(RedBox, text=message, font='Montserrat 10', bg =red, fg='black').place(x=362, y=(List[listCount]))
    tk.Label(RedBox, text=num, font='Montserrat 10', bg=red, fg='black').place(x=743, y=(List[listCount]))
    listCount += 1

def orangeBox(message, num):
    global listCount
    if (listCount > 5):
        listCount = 0
    OrangeBox = tk.Canvas(root, width=455, height=40, bg=orange, highlightthickness=0).place(x=355, y=List[listCount])
    tk.Label(OrangeBox, text=message, font='Montserrat 10', bg=orange, fg='black').place(x=362, y=(List[listCount]))
    tk.Label(OrangeBox, text=num, font='Montserrat 10', bg=orange, fg='black').place(x=743, y=(List[listCount]))
    listCount += 1

def yellowBox(message, num):
    global listCount
    if (listCount > 5):
        listCount = 0
    YellowBox = tk.Canvas(root, width=455, height=40, bg=yellow, highlightthickness=0).place(x=355, y=List[listCount])
    tk.Label(YellowBox, text=message, font='Montserrat 10', bg=yellow, fg='black').place(x=362, y=(List[listCount]))
    tk.Label(YellowBox, text=num, font='Montserrat 10', bg=yellow, fg='black').place(x=743, y=(List[listCount]))
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
db.get_message_by_name('battery_vt').frame_id : [(nothing, 'voltage'),(nothing, 'current'),(nothing, 'temperature'),(nothing, 'batt_perc')],
db.get_message_by_name('battery_status').frame_id : [(BMS_Fault, 'fault'),(nothing, 'aux_batt_v')],
db.get_message_by_name('pd_status').frame_id : [(nothing, 'power_state'), (display_pd_status, 'fault_bitset'), (nothing, 'bps_persist')], #something for fault
db.get_message_by_name('mc_status').frame_id : [(nothing, 'limit_bitset_l'),(mc_error_bitset_l, 'error_bitset_l'),(nothing, 'limit_bitset_l'),
(mc_error_bitset_r, 'error_bitset_r'),(nothing, 'board_fault_bitset'),
(nothing, 'precharge_status')]
}


def handle_message(msg):
    global speedCount

    try:
        combinedSpeed = 0
        decoded_message = db.decode_message(msg.arbitration_id, msg.data)
        #print(decoded_message)
        if msg.arbitration_id in DISPLAY_MSG_DICT.keys():
            for sig in DISPLAY_MSG_DICT[msg.arbitration_id]:
                if (sig[1] == 'fault'):
                    print("msg.data: ", msg.data)
                    print(''.join(hex(byte)[2:].zfill(2) for byte in msg.data))
                    sig[0](decoded_message)
                else:
                    sig[0](decoded_message[sig[1]])
                    if (sig[1] == 'batt_perc'):
                        tk.Label(BatteryBox, text=(decoded_message[sig[1]], "%"), width=5, font='Montserrat 25', bg='#1b2838', fg='white').place(x=220, y=17)
                    if (sig[1] == 'target_velocity'):
                        tk.Label(TargetSpeedBox, text=(decoded_message[sig[1]], "KH/H"), width=5, font='Montserrat 25', bg='#1b2838', fg='white').place(x=220, y=197)
                    if (sig[1] == 'velocity_l'):
                        combinedSpeed += decoded_message[sig[1]]
                    speedCount += 1
                    if (sig[1] == 'velocity_r'):
                        combinedSpeed += decoded_message[sig[1]]
                    speedCount += 1
                    if (sig[1] == 'temperature'):
                        tk.Label(TempBox, text=(decoded_message[sig[1]], "C"), width=5, font='Montserrat 25', bg='#1b2838', fg='white').place(x=220, y=77)
                    if (speedCount == 2):
                        tk.Label(SpeedBox, text=((combinedSpeed/2), "KM/H"), width=5, font='Montserrat 25', bg='#1b2838', fg='white').place(x=220, y=137)
                    speedCount = 0
    except KeyError:
        print("Unknown message:", msg)
# update_displays()

notifier = can.Notifier(can_bus, [handle_message])

while True:
    root.mainloop()