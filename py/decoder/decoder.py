import os
import pty
import cantools
import serial

class State:
    SOM = "SOM"
    ID = "ID"
    DLC = "DLC"
    DATA = "DATA"
    EOM = "EOM"
    VALID = "VALID"

class DatagramDecoder:
    def __init__(self):
        self.message_state = State.SOM
        self.buffer = []
        self.datagram = None
        self.init_serial()
        self.init_dbc()

    def init_serial(self):
        self.master, self.slave = pty.openpty()
        self.slave_name = os.ttyname(self.slave)
        self.slave_serial = serial.Serial(self.slave_name)

    def init_dbc(self):
        dbc_path = "/Users/ericgao/box/shared/fwxv/libraries/codegen/system_can.dbc"
        self.db = cantools.database.load_file(dbc_path)

    def write(self, packet):
        self.slave_serial.write(bytes(packet))

    def read(self):
        recv = os.read(self.master,1000)
        if self.parse_byte(recv):
            message = self.db.get_message_by_frame_id(self.datagram["id"])
            decoded_data = message.decode(self.datagram['data'])
            print(decoded_data)

    def read_test(self, byte):
        if self.parse_byte(byte):
            message = self.db.get_message_by_frame_id(self.datagram["id"])
            decoded_data = message.decode(bytes(self.datagram['data']))
            print(decoded_data)

    def is_valid_id(self, id):
        try:
            self.db.get_message_by_frame_id(id)
            return True
        except KeyError:
            return False

    def parse_byte(self, byte):
        match self.message_state:
            case (State.SOM | State.VALID):
                self.buffer = []
                self.datagram = None
                if byte == 0xAA:
                    self.message_state = State.ID
            case State.ID:
                self.buffer.append(byte)
                if len(self.buffer) == 4:
                    message_id = int.from_bytes(self.buffer, byteorder='big')
                    if self.is_valid_id(message_id):
                        self.datagram = {"id": message_id}
                        self.buffer = []
                        self.message_state = State.DLC
                    else:
                        self.message_state = State.SOM
            case State.DLC:
                self.datagram["dlc"] = byte
                if self.datagram["dlc"] <= 9:
                    self.datagram["data"] = []
                    self.message_state = State.DATA
                else:
                    self.message_state = State.SOM
            case State.DATA:
                self.buffer.append(byte)
                if len(self.buffer) == self.datagram["dlc"]:
                    self.datagram["data"] = self.buffer
                    self.message_state = State.EOM
            case State.EOM:
                if byte == 0xBB:
                    self.message_state = State.VALID
                else:
                    self.message_state = State.SOM
        return self.message_state == State.VALID
    