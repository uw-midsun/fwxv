'''This client script handles datagram protocol communication between devices on the CAN'''

<<<<<<< HEAD
=======
import zlib
>>>>>>> bootloader datagram setup
import can

DEFAULT_CHANNEL = 'can0'
CAN_BITRATE = 500000

<<<<<<< HEAD
<<<<<<< HEAD
DATA_SIZE_SIZE = 2
MIN_BYTEARRAY_SIZE = 5

DATAGRAM_TYPE_OFFSET = 0
=======
MESSAGE_SIZE = 8
HEADER_SIZE = 6
=======
>>>>>>> datagram class complete with test cases
DATA_SIZE_SIZE = 2
MIN_BYTEARRAY_SIZE = 9

DATAGRAM_TYPE_OFFSET = 0
CRC_32_OFFSET = 1
>>>>>>> bootloader datagram setup
NODE_IDS_OFFSET = 5
DATA_SIZE_OFFSET = 7

CAN_START_ARBITRATION_ID = 0b00000010000
CAN_ARBITRATION_ID = 0b00000000000


class DatagramTypeError(Exception):
<<<<<<< HEAD
    # pylint: disable=unnecessary-pass
=======
>>>>>>> bootloader datagram setup
    '''Error wrapper (NEEDS WORK)'''
    pass


class Datagram:
    '''Custom CAN-datagram class'''
<<<<<<< HEAD
<<<<<<< HEAD

=======
>>>>>>> bootloader datagram setup
=======

>>>>>>> datagram class complete with test cases
    def __init__(self, **kwargs):
        '''Initialize datagram class'''
        self._check_kwargs(**kwargs)
        self._datagram_type_id = kwargs["datagram_type_id"] & 0xff

        self._node_ids = []
        for val in kwargs["node_ids"]:
            self._node_ids.append(val & 0xff)

        self._data = kwargs["data"]

    @classmethod
<<<<<<< HEAD
<<<<<<< HEAD
    def _unpack(cls, datagram_bytearray):
        '''This function returns an instance fo the class by unpacking a bytearray'''
        assert isinstance(datagram_bytearray, bytearray)

=======
    def unpack(cls, datagram_bytearray):
=======
    def _unpack(cls, datagram_bytearray):
>>>>>>> datagram class complete with test cases
        '''This function returns an instance fo the class by unpacking a bytearray'''
        assert isinstance(datagram_bytearray, bytearray)

        # "theoretical" lower limit:
        # 1 (type) + 4 (crc32) + 2 (node ids) + 2 (data size) + 0 (data)
        #   = 9
>>>>>>> bootloader datagram setup
        if len(datagram_bytearray) < MIN_BYTEARRAY_SIZE:
            raise DatagramTypeError(
                "Invalid Datagram format from bytearray: Does not meet minimum size requirement")

        datagram_type_id = datagram_bytearray[DATAGRAM_TYPE_OFFSET]
<<<<<<< HEAD
        node_ids_raw = datagram_bytearray[NODE_IDS_OFFSET:DATA_SIZE_OFFSET]
        node_ids = cls._unpack_nodeids(cls, node_ids_raw)
        data_size = cls._convert_from_bytearray(
            datagram_bytearray[DATA_SIZE_OFFSET:DATA_SIZE_OFFSET + 2], 2)
=======
        crc32 = datagram_bytearray[CRC_32_OFFSET:NODE_IDS_OFFSET]
        node_ids_raw = datagram_bytearray[NODE_IDS_OFFSET:DATA_SIZE_OFFSET]
<<<<<<< HEAD
        # Gets a list of node ID's from message
        node_ids = []
        while node_ids_raw:
            count = 0
            byte = node_ids_raw & (~node_ids_raw + 1)
            node_ids_raw ^= byte
            while byte:
                byte = byte >> 1
                count += 1
            node_ids.append(count)

        data_size = cls._convert_from_bytearray(datagram_bytearray[DATA_SIZE_OFFSET], 2)
>>>>>>> bootloader datagram setup
=======
        node_ids = cls._unpack_nodeids(cls, node_ids_raw)
        data_size = cls._convert_from_bytearray(datagram_bytearray[DATA_SIZE_OFFSET:DATA_SIZE_OFFSET+2], 2)
>>>>>>> datagram class complete with test cases

        if len(datagram_bytearray) != MIN_BYTEARRAY_SIZE + data_size:
            raise DatagramTypeError("Invalid Datagram format from bytearray: Not enough data bytes")

        data = datagram_bytearray[DATA_SIZE_OFFSET + DATA_SIZE_SIZE:]
        exp_crc32 = cls._calculate_crc32(cls, datagram_type_id, node_ids_raw, data)

<<<<<<< HEAD
<<<<<<< HEAD
        return cls(datagram_type_id=datagram_type_id, node_ids=node_ids, data=data)

    def pack(self):
        '''This function packs a new bytearray based on set data'''
        node_ids = self._pack_nodeids(self._node_ids)

        return bytearray([
            self._datagram_type_id,
            *node_ids,
            len(self._data) & 0xff,
            (len(self._data) >> 8) & 0xff,
            *(self._data)
        ])
=======
    def pack(self):
        '''This function packs a new bytearray based on set data'''
        print("INCOMPLETE")
>>>>>>> bootloader datagram setup
=======
        crc32 = cls._convert_from_bytearray(crc32, 4)

        if (exp_crc32 != crc32):
            raise DatagramTypeError("Invalid crc32")

        return cls(datagram_type_id=datagram_type_id, node_ids=node_ids, data=data)

    def _pack(self):
        '''This function packs a new bytearray based on set data'''
        node_ids = self._pack_nodeids(self._node_ids)

        crc32 = self._calculate_crc32(self._datagram_type_id, node_ids, self._data)
        crc32 = self._convert_to_bytearray(crc32, 4)

        return bytearray([
            self._datagram_type_id,
            *crc32,
            *node_ids,
            len(self._data) & 0xff,
            (len(self._data) >> 8) & 0xff,
            *(self._data)
        ])
>>>>>>> datagram class complete with test cases

    @property
    def datagram_type_id(self):
        '''This function describe the datragram id'''
        return self._datagram_type_id

    @property
    def node_ids(self):
        '''This function describe the datragram id'''
        return self._node_ids

    @property
    def data(self):
        '''This function describe the datragram id'''
        return self._data

    @datagram_type_id.setter
    def datagram_type_id(self, value):
        '''This function sets the datragram id'''
        assert value & 0xff == value
        self._datagram_type_id = value & 0xff

    @node_ids.setter
    def node_ids(self, nodes):
        '''This function sets the datragram id'''
        assert isinstance(nodes, list)
        assert all(0 <= nodes < 0xff for node in nodes)
        self._node_ids = nodes

    @data.setter
    def data(self, value):
        '''This function sets the datragram id'''
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

<<<<<<< HEAD
<<<<<<< HEAD
        assert not isinstance(kwargs["datagram_type_id"], list)
=======
        assert not isinstance(kwargs["datagram_type_id", list])
>>>>>>> bootloader datagram setup
=======
        assert not isinstance(kwargs["datagram_type_id"], list)
>>>>>>> datagram class complete with test cases
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
<<<<<<< HEAD
<<<<<<< HEAD

=======
>>>>>>> datagram class complete with test cases
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

<<<<<<< HEAD

class DatagramSender:
    # pylint: disable=too-few-public-methods
    '''Class that acts as a distributor for the Datagram class on a CAN bus'''

    def __init__(self, bustype="socketcan", channel=DEFAULT_CHANNEL,
                 bitrate=CAN_BITRATE, receive_own_messages=False):
        # pylint: disable=abstract-class-instantiated
=======
    def _calculate_crc32(self, datagram_type_id, node_ids, data):
        '''This function returns a crc32 calculation'''
        node_crc32 = zlib.crc32(bytearray(node_ids))
        node_crc32 = self._convert_to_bytearray(node_crc32, 4)
        data_crc32 = zlib.crc32(bytearray(data))
        data_crc32 = self._convert_to_bytearray(data_crc32, 4)
        
        crc32_array = bytearray([datagram_type_id,
                                 len(node_ids),
                                 * node_crc32,
                                 len(data) & 0xff,
                                 (len(data) >> 8) & 0xff,
                                 *data_crc32
                                 ])
        crc32 = zlib.crc32(crc32_array)
        return crc32

class DatagramSender:
    '''Class that acts as a distributor for the Datagram class on a CAN bus'''
    def __init__(self, bustype="socketcan", channel=DEFAULT_CHANNEL, bitrate=CAN_BITRATE, receive_own_messages=False):
>>>>>>> datagram class complete with test cases
        self.bus = can.interface.Bus(
            bustype=bustype,
            channel=channel,
            bitrate=bitrate,
            receive_own_messages=receive_own_messages)
<<<<<<< HEAD

    def send(self, message, sender_id=0):
        '''Send a Datagram over CAN'''
        assert isinstance(message, Datagram)
        chunk_messages = self._chunkify(message.pack(), 8)
        message_extended_arbitration = False

        start_arbitration_board_id = (CAN_START_ARBITRATION_ID | sender_id << 5)
        arbitration_board_id = (CAN_ARBITRATION_ID | sender_id << 5)

        can_messages = [
            can.Message(
                arbitration_id=start_arbitration_board_id,
                data=bytearray(),
                is_extended_id=message_extended_arbitration)]

        # Populate an array with the can message from the library
        for chunk_message in chunk_messages:
            can_messages.append(can.Message(arbitration_id=arbitration_board_id,
                                            data=chunk_message,
                                            is_extended_id=message_extended_arbitration))

        for msg in can_messages:
            self.bus.send(msg)
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
        # Messages are stored in a dictionary where key = board ID, value = message
        self.datagram_messages = {}
        super().__init__()

    def on_message_received(self, msg: can.Message):
        '''Handles message sent from boards on the CAN'''
        super().on_message_received(msg)

        board_id = (msg.arbitration_id & 0b11111100000) >> 5
        start_message = (msg.arbitration_id & 0x10) >> 4

        if start_message == 1:
            # Reset the datagram message when receiving a start message
            self.datagram_messages[board_id] = msg.data

        if start_message == 0:
            if board_id in self.datagram_messages:
                self.datagram_messages[board_id] += msg.data

        try:
            datagram = Datagram._unpack(self.datagram_messages[board_id])
        except DatagramTypeError:
            # Datagram is incomplete, continue until complete
            pass
        else:
            # Datagram is complete, call the callback with formed datagram
            self.callback(datagram, board_id)
=======
>>>>>>> bootloader datagram setup
=======
    
    def send(self, message, sender_id=0):
        '''Send a Datagram over CAN'''
        assert isinstance(message, Datagram)

class DatagramListener:
    '''Class that acts as a listener for the Datagram class on a CAN bus'''
>>>>>>> datagram class complete with test cases