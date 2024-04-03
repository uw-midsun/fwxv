'''This client script handles datagram protocol communication between devices on the CAN'''

import zlib
import can

DEFAULT_CHANNEL = 'can0'
CAN_BITRATE = 500000

MESSAGE_SIZE = 8
HEADER_SIZE = 6
DATA_SIZE_SIZE = 2
MIN_BYTEARRAY_SIZE = 9

DATAGRAM_TYPE_OFFSET = 0
CRC_32_OFFSET = 1
NODE_IDS_OFFSET = 5
DATA_SIZE_OFFSET = 7

CAN_START_ARBITRATION_ID = 0b00000010000
CAN_ARBITRATION_ID = 0b00000000000


class DatagramTypeError(Exception):
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
    def unpack(cls, datagram_bytearray):
        '''This function returns an instance fo the class by unpacking a bytearray'''
        assert isinstance(datagram_bytearray, bytearray)

        # "theoretical" lower limit:
        # 1 (type) + 4 (crc32) + 2 (node ids) + 2 (data size) + 0 (data)
        #   = 9
        if len(datagram_bytearray) < MIN_BYTEARRAY_SIZE:
            raise DatagramTypeError(
                "Invalid Datagram format from bytearray: Does not meet minimum size requirement")

        datagram_type_id = datagram_bytearray[DATAGRAM_TYPE_OFFSET]
        crc32 = datagram_bytearray[CRC_32_OFFSET:NODE_IDS_OFFSET]
        node_ids_raw = datagram_bytearray[NODE_IDS_OFFSET:DATA_SIZE_OFFSET]
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

        if len(datagram_bytearray) != MIN_BYTEARRAY_SIZE + data_size:
            raise DatagramTypeError("Invalid Datagram format from bytearray: Not enough data bytes")

        data = datagram_bytearray[DATA_SIZE_OFFSET + DATA_SIZE_SIZE:]

    def pack(self):
        '''This function packs a new bytearray based on set data'''
        print("INCOMPLETE")

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

        assert not isinstance(kwargs["datagram_type_id", list])
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
