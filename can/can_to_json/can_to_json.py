import can
import cantools
from datetime import datetime
from mongodb_api import *
from multiprocessing.connection import Listener
import os

# Load DBC file
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
db = cantools.database.load_file(SCRIPT_DIR + '/../tools/system_can.dbc')

# Set up connection to MongoDB
client = init_mongodb()
database = client["systemcan"]

# Create a new MongoDB collection
now = datetime.now()
collection_name = now.strftime("%m/%d/%y %H:%M:%S")
collection = database[collection_name]

# Dictionary initialization
can_messages_dict = {}

for message in db.messages: 
    message_dict = {
        'name': message.name,
        'frame_id': message.frame_id,
        'signals': {}
    }

    for signal in message.signals: 
        message_dict['signals'][signal.name] = 0

    can_messages_dict[str(message.frame_id)] = message_dict

# can_messages_dict['GPS'] = {}  # TODO: Get GPS data 

dt_string = now.strftime("%m/%d/%y %H:%M:%S")
can_messages_dict['timestamp'] = dt_string

upload_mongodb(can_messages_dict, collection)

# Multiprocess listener
# address = ('localhost', 6000)
# listener = Listener(address, authkey=b'secret password')
# conn = listener.accept()
# print('connection accepted from', listener.last_accepted)

# while True:
#     try:
#         # Assume data from GUI is sent as a tuple (arbitration_id, {data_dict})
#         id, data_dict = conn.recv()
#         if conn is not None:
#             # Update can_messages_dict
#             for signal in data_dict.keys():
#                 can_messages_dict[id]['signals'][signal] = data_dict[signal]

#         # Update dict and upload to MongoDB
#         can_messages_dict['GPS'] = {}  # TODO: Get GPS data 

#         now = datetime.now()
#         dt_string = now.strftime("%m/%d/%y %H:%M:%S")
#         can_messages_dict['timestamp'] = dt_string
#         upload_mongodb(can_messages_dict, collection)
        
#         conn.close()
#         conn = listener.accept()
#     except Exception as e:
#         print("Error: ", e)
#         continue


# TODO: Replace sniffer with IPC
os.system("sudo ip link add dev vcan0 type vcan")
os.system("sudo ip link set up vcan0")

can_bus = can.interface.Bus(channel="vcan0", bustype="socketcan")

def listener(msg):
    try: 
        print("Listener Executed")
        decoded_message = db.decode_message(msg.arbitration_id, msg.data)
        print(decoded_message)
        for signal in decoded_message.keys():
            can_messages_dict[str(msg.arbitration_id)]['signals'][signal] = decoded_message[signal]
        
        now = datetime.now()
        dt_string = now.strftime("%m/%d/%y %H:%M:%S")
        can_messages_dict['timestamp'] = dt_string

        upload_mongodb(can_messages_dict, collection)

    except KeyError:
        print("Unknown message: ", msg)

notifier = can.Notifier(can_bus, [listener])

print("Setup Complete")

while True: 
    continue
# End of sniffer
