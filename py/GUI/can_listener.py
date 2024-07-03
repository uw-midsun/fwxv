import can
import cantools
from datetime import datetime
import GPS
import mongodb_api
import multiprocessing
from multiprocessing import Manager, Process
from multiprocessing.connection import Listener
import os
import serial
import time

def updateDict(ns, can_messages_dict):
     # Multiprocess listener
    address = ('localhost', 6000)
    listener = Listener(address, authkey=b'secret password')
    conn = listener.accept()
    print('connection accepted from', listener.last_accepted)
    
    while True:
        try:
            # Assume data from GUI is sent as a tuple (arbitration_id, {data_dict})
            msg_id, data_dict = conn.recv()
            print(msg_id)
            print(data_dict)
            if conn is not None:
                print("Data is ready")
                # Update can_messages_dict
                for signal in data_dict.keys():
                    can_messages_dict[str(msg_id)]['signals'][signal] = data_dict[signal]
                ns.x = 1
        except Exception as e:
            print("Error: ", e)
            time.sleep(1)
            conn.close()
            conn = listener.accept()
            continue

def uploadDict(ns, can_messages_dict):    
    while True:
        if ns.x == 1:
            ns.x = 0
            print("Data received and uploading")
            rec_buff = GPS.get_gps_position(ser)
            can_messages_dict['GPS'] = rec_buff

            now = datetime.now()
            dt_string = now.strftime("%m/%d/%y %H:%M:%S")
            can_messages_dict['timestamp'] = dt_string
            mongodb_api.upload_mongodb(to_dict(can_messages_dict), collection)
            print(to_dict(can_messages_dict))
        
        # Upload dict to mongoDB at most once per 5 seconds
        time.sleep(5)

def to_dict(d):
    return {
        key: to_dict(sub_d)
        if isinstance(sub_d, multiprocessing.managers.DictProxy) else sub_d
        for key, sub_d in d.items()
            }

# Load DBC file
db = cantools.database.load_file('/home/midnightsun/GUI/system_can.dbc')

# Set up connection to MongoDB
client = mongodb_api.init_mongodb()
database = client["systemcan"]

# Create a new MongoDB collection
now = datetime.now()
collection_name = now.strftime("%m/%d/%y %H:%M:%S")
collection = database[collection_name]

# Initialize SIM7600X GPS Module
ser = serial.Serial('/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0',115200)
ser.reset_input_buffer()
power_key = 6
rec_buff = ''
time_count = 0
GPS.power_on(ser, power_key)

manager = Manager()

# Dictionary initialization
can_messages_dict = manager.dict()

for message in db.messages: 
    message_dict = manager.dict({
        'name': message.name,
        'frame_id': message.frame_id,
        'signals': manager.dict()
    })

    for signal in message.signals: 
        message_dict['signals'][signal.name] = 0

    can_messages_dict[str(message.frame_id)] = message_dict

# Get GPS data using AT+CGPSINFO
rec_buff = GPS.get_gps_position(ser)
can_messages_dict['GPS'] = rec_buff
print(rec_buff)

# Get current date and time
dt_string = now.strftime("%m/%d/%y %H:%M:%S")
can_messages_dict['timestamp'] = dt_string

# Upload dictionary template
mongodb_api.upload_mongodb(to_dict(can_messages_dict), collection)


# Run updateDict() and uploadDict() in parallel
ns = manager.Namespace()
# ns.x shows if data is ready to be received and uploaded
ns.x = 0

# Start processes
update = Process(target=updateDict, args=(ns, can_messages_dict))
upload = Process(target=uploadDict, args=(ns, can_messages_dict))

update.start()
upload.start()

update.join()
upload.join()
