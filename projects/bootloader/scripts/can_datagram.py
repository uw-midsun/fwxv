'''This client script handles datagram protocol communication between devices on the CAN'''

import can
import time
from crc32 import CRC32
from bootloader_id import *

DEFAULT_CHANNEL = 'can0'
CAN_BITRATE = 1000000

DATA_SIZE_SIZE = 2
MIN_BYTEARRAY_SIZE = 4

NODE_IDS_OFFSET = 0
DATA_SIZE_OFFSET = 2

crc32 = CRC32(STANDARD_CRC32_POLY)

class DatagramTypeError(Exception):
    # pylint: disable=unnecessary-pass
    '''Error wrapper (NEEDS WORK)'''
    pass


class Datagram:
    '''Custom CAN-datagram class'''

    def __init__(self, **kwargs):
        '''Initialize datagram class'''
        self._check_kwargs(**kwargs)
        self._datagram_type_id = kwargs["datagram_type_id"] & 0xff

        self._node_ids = []
        for val in kwargs["node_ids"]:
            self._node_ids.append(val & 0xff)

        self._data = kwargs["data"]

    @classmethod
    def _unpack(cls, datagram_bytearray, arbitration_id):
        '''This function returns an instance fo the class by unpacking a bytearray'''
        assert isinstance(datagram_bytearray, bytearray)
        if len(datagram_bytearray) < MIN_BYTEARRAY_SIZE:

            raise DatagramTypeError(
                "Invalid Datagram format from bytearray: Does not meet minimum size requirement")

        if arbitration_id == START:
            node_ids_raw = datagram_bytearray[NODE_IDS_OFFSET:DATA_SIZE_OFFSET]
            node_ids = cls._unpack_nodeids(cls, node_ids_raw)
            return cls(datagram_type_id=arbitration_id, node_ids=node_ids, data=bytearray())
        elif arbitration_id == FLASH:
            data = []
            data = datagram_bytearray
            return cls(datagram_type_id=arbitration_id, node_ids=[], data=data)

    def pack(self):
        '''This function packs a new bytearray based on set data'''
        node_ids = self._pack_nodeids(self._node_ids)

        return bytearray([
            *node_ids
        ])

    @property
    def datagram_type_id(self):
        '''This function describes the datagram id'''
        return self._datagram_type_id

    @property
    def node_ids(self):
        '''This function assigns the datagram id'''
        return self._node_ids

    @property
    def data(self):
        '''This function describe the datagram node ids'''
        return self._data

    @datagram_type_id.setter
    def datagram_type_id(self, value):
        '''This function sets the datagram type'''
        assert value & 0xff == value
        self._datagram_type_id = value & 0xff

    @node_ids.setter
    def node_ids(self, nodes):
        '''This function sets the datagram node ids'''
        assert isinstance(nodes, list)
        assert all(0 <= nodes < 0xff for node in nodes)
        self._node_ids = nodes

    @data.setter
    def data(self, value):
        '''This function sets the datagram data'''
        assert isinstance(value, bytearray)
        self._data = value

    @staticmethod
    def _check_kwargs(**kwargs):
        '''This function checks that all args passed in are correct'''

        args = [
            "datagram_type_id",
            "node_ids",
            "data"
        ]

        for arg in args:
            assert arg in kwargs

        assert not isinstance(kwargs["datagram_type_id"], list)
        assert isinstance(kwargs["node_ids"], list)
        assert isinstance(kwargs["data"], bytearray)

        assert kwargs["datagram_type_id"] & 0xff == kwargs["datagram_type_id"]

    @staticmethod
    def _convert_from_bytearray(in_bytearray, size):
        '''Helper function to get little endian value from byte array'''
        out_value = 0
        for i in range(size):
            out_value = out_value | ((in_bytearray[i] & 0xff) << (i * 8))
        return out_value

    @staticmethod
    def _convert_to_bytearray(in_value, size):
        '''Helper function to get a little endian byte array from a value'''
        out_array = bytearray()
        for i in range(size):
            out_array.append((in_value >> (8 * i)) & 0xff)
        return out_array

    def _unpack_nodeids(self, raw_nodes):
        '''This function takes in the raw bytearray of node ids and returns an ordered list'''
        node_vals = self._convert_from_bytearray(raw_nodes, 2)
        # Gets a list of node ID's from message
        out_nodeids = []
        while node_vals:
            count = 0
            byte = node_vals & (~node_vals + 1)
            node_vals ^= byte
            while byte:
                byte = byte >> 1
                count += 1
            out_nodeids.append(count)

        return out_nodeids

    def _pack_nodeids(self, raw_nodes):
        '''This function takes in an ordered list and outputs a bytearray of node ids'''
        out_value = 0
        for node in raw_nodes:
            out_value = (out_value) | (1 << (node - 1))
        out_value = self._convert_to_bytearray(out_value, DATA_SIZE_OFFSET - NODE_IDS_OFFSET)
        return out_value


class DatagramSender:
    # pylint: disable=too-few-public-methods
    '''Class that acts as a distributor for the Datagram class on a CAN bus'''

    def __init__(self, bustype="socketcan", channel=DEFAULT_CHANNEL,
                 bitrate=CAN_BITRATE, receive_own_messages=False):
        # pylint: disable=abstract-class-instantiated
        self.bus = can.interface.Bus(
            bustype=bustype,
            channel=channel,
            bitrate=bitrate,
            receive_own_messages=receive_own_messages)

    def send(self, message, sender_id=0):
        '''Send a Datagram over CAN'''
        assert isinstance(message, Datagram)
        datagram = message.pack()
        datagram.extend(message.data)

        message_extended_arbitration = False
        can_message = can.Message(arbitration_id=message._datagram_type_id,
                                  data=datagram,
                                  is_extended_id=message_extended_arbitration)
        self.bus.send(can_message)
        print(can_message)
        print("Message was sent on {}".format(self.bus.channel_info))

        ack_received = False
        retry_count = 0
        max_retries = 3
        
        while not ack_received and retry_count < max_retries:
            try:
                ack_msg = self.bus.recv(timeout=5.0)
                
                if ack_msg and ack_msg.arbitration_id == ACK:
                    if ack_msg.data[0] == 0x01:
                        ack_received = True
                        print(f"Received ACK for start message")
                    elif ack_msg.data[0] == 0x00:
                        print(f"Received NACK for start message, aborting")
                        break
                    else:
                        print(f"Received unknown response for start message, retrying...")
                        retry_count += 1
                else:
                    print(f"No ACK/NACK received for start message, retrying...")
                    retry_count += 1
                
            except can.CanError:
                print(f"Error waiting for ACK/NACK for start message, retrying...")
                retry_count += 1
        
        if not ack_received:
            raise Exception(f"Failed to receive ACK for start message after {max_retries} attempts")
    

        print(f"Start message received succesfully!")

    def send_data(self, message, sender_id=0):
        '''Send a Datagram over CAN'''
        assert isinstance(message, Datagram)
        start_time = time.time()
        message_extended_arbitration = False
        chunk_messages = list(self._chunkify(message.data, 8))
        sequence_number = 0
        
        while chunk_messages:
            seq_num_bytes = sequence_number.to_bytes(2, byteorder='little')
            
            # Prepare up to 1024 bytes (128 chunks of 8 bytes each)
            current_chunk = chunk_messages[:128]
            chunk_messages = chunk_messages[128:]
            
            crc_chunk = b''.join(current_chunk)
            crc32_value = crc32.calculate(crc_chunk)
            crc_data = crc32_value.to_bytes(4, byteorder='little')
            
            sequencing_data = seq_num_bytes + crc_data
            
            # Send sequence message
            sequence_msg = can.Message(arbitration_id=SEQUENCING,
                                    data=sequencing_data,
                                    is_extended_id=message_extended_arbitration)
            
            self.bus.send(sequence_msg)
            
            # Send data chunks (up to 1024 bytes)
            for chunk in current_chunk:
                try:
                    data_msg = can.Message(arbitration_id=FLASH,
                                        data=chunk,
                                        is_extended_id=message_extended_arbitration)
                    self.bus.send(data_msg)
                except BaseException:
                    time.sleep(0.01)
                    self.bus.send(data_msg)
            
            print(f"Sent {len(current_chunk) * 8} bytes for sequence {sequence_number}\n")
            
            if sequence_number > 0 or chunk_messages:
                ack_received = False
                retry_count = 0
                max_retries = 3
                
                while not ack_received and retry_count < max_retries:
                    try:
                        ack_msg = self.bus.recv(timeout=5.0)
                        
                        if ack_msg and ack_msg.arbitration_id == ACK:
                            if ack_msg.data[0] == 0x01:
                                ack_received = True
                                print(f"Received ACK for sequence {sequence_number}")
                            elif ack_msg.data[0] == 0x00:
                                print(f"Received NACK for sequence {sequence_number}, retrying...")
                                retry_count += 1
                                break
                            else:
                                print(f"Received unknown response for sequence {sequence_number}, retrying...")
                                retry_count += 1
                        else:
                            print(f"No ACK/NACK received for sequence {sequence_number}, retrying...")
                            retry_count += 1
                        
                    except can.CanError:
                        print(f"Error waiting for ACK/NACK for sequence {sequence_number}, retrying...")
                        retry_count += 1
                
                if not ack_received:
                    raise Exception(f"Failed to receive ACK for sequence {sequence_number} after {max_retries} attempts")
            
            sequence_number += 1
        
        end_time = time.time()

        print(f"Time Elapsed: {end_time - start_time}")
        print(f"All data sent successfully. Total sequences: {sequence_number}\n")

    @staticmethod
    def _chunkify(data, size):
        '''This chunks up the datagram bytearray for easy iteration'''
        return (data[pos:pos + size] for pos in range(0, len(data), size))


class DatagramListener(can.BufferedReader):
    # pylint: disable=too-few-public-methods
    '''Class that acts as a listener for the Datagram class on a CAN bus'''

    def __init__(self, callback):
        '''Registers the callback'''
        assert callable(callback)
        self.callback = callback
        self.board_ids = 0
        # self.callback("1", 10)
        # Messages are stored in a dictionary where key = board ID, value = message
        self.datagram_messages = {}
        super().__init__()

    def on_message_received(self, msg: can.Message):
        '''Handles message sent from boards on the CAN'''
        super().on_message_received(msg)
        arbitration_id = (msg.arbitration_id & 0xff)

        if arbitration_id == START:
            self.board_ids = self.extract_board_id(msg.data)
        for board_id in self.board_ids:
            if arbitration_id == START:
                # Reset the datagram message when receiving a start message
                self.datagram_messages[board_id] = msg.data

            if arbitration_id != START:
                if board_id in self.datagram_messages:
                    self.datagram_messages[board_id] += msg.data

            try:
                datagram = Datagram._unpack(self.datagram_messages[board_id], arbitration_id)
                self.callback(msg, board_id)
            except DatagramTypeError:
                # Datagram is incomplete, continue until complete
                pass

    def extract_board_id(self, data):
        '''Extracts the board ID from the data'''
        out_value = 0
        for i in range(2):
            out_value = out_value | ((data[i] & 0xff) << (i * 8))

        out_nodeids = []
        while out_value:
            count = 0
            byte = out_value & (~out_value + 1)
            out_value ^= byte
            while byte:
                byte = byte >> 1
                count += 1
            out_nodeids.append(count)

        return out_nodeids
