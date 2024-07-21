'''This client script handles datagram protocol communication between devices on the CAN'''

import can

DEFAULT_CHANNEL = 'can0'
CAN_BITRATE = 500000

DATA_SIZE_SIZE = 2
MIN_BYTEARRAY_SIZE = 4

NODE_IDS_OFFSET = 0
DATA_SIZE_OFFSET = 2

CAN_START_ARBITRATION_ID = 30
CAN_ARBITRATION_FLASH_ID = 31


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

        if arbitration_id == CAN_START_ARBITRATION_ID:
            node_ids_raw = datagram_bytearray[NODE_IDS_OFFSET:DATA_SIZE_OFFSET]
            node_ids = cls._unpack_nodeids(cls, node_ids_raw)
            return cls(datagram_type_id=arbitration_id, node_ids=node_ids, data=bytearray())
        elif arbitration_id == CAN_ARBITRATION_FLASH_ID:
            data = []
            data = datagram_bytearray
            return cls(datagram_type_id=arbitration_id, node_ids=[], data=data)


    def pack(self):
        '''This function packs a new bytearray based on set data'''
        node_ids = self._pack_nodeids(self._node_ids)

        return bytearray([
            *node_ids,
            len(self._data) & 0xff,
            (len(self._data) >> 8) & 0xff,
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
        print("HERE")
        # pylint: disable=abstract-class-instantiated
        self.bus = can.interface.Bus(
            bustype=bustype,
            channel=channel,
            bitrate=bitrate,
            receive_own_messages=receive_own_messages)

    def send(self, message, sender_id=0):
        '''Send a Datagram over CAN'''
        assert isinstance(message, Datagram)
        start_message = message.pack()
        chunk_messages = list(self._chunkify(message.data, 8))
        
        message_extended_arbitration = False
        can_messages = [can.Message(arbitration_id=CAN_START_ARBITRATION_ID,
                        data=start_message,
                        is_extended_id=message_extended_arbitration)]

        # Populate an array with the can message from the library
        for chunk_message in chunk_messages:
            can_messages.append(can.Message(arbitration_id=message._datagram_type_id,
                                            data=chunk_message,
                                            is_extended_id=message_extended_arbitration))

        for msg in can_messages:
            self.bus.send(msg)
            print(msg)
        print("{} messages were sent on {}".format(len(can_messages), self.bus.channel_info))

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

        if arbitration_id == CAN_START_ARBITRATION_ID:
            self.board_ids = self.extract_board_id(msg.data)
        for board_id in self.board_ids:
            if arbitration_id == CAN_START_ARBITRATION_ID:
                # Reset the datagram message when receiving a start message
                self.datagram_messages[board_id] = msg.data

            if arbitration_id != CAN_START_ARBITRATION_ID:
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