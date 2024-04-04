# pylint: skip-file
'''This Module Tests methods in can_datagram.py'''
import unittest
import random

from can_datagram import Datagram, DatagramSender, DatagramListener
import can

TEST_CHANNEL = "vcan0"

TEST_DATAGRAM_TYPE_ID = 1
TEST_NODES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
TEST_NODES_RAW1 = 0b11111111
TEST_NODES_RAW2 = 0b00000011
TEST_DATA = bytearray([3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9,
                       7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3])


class TestCanDatagram(unittest.TestCase):
    '''Test Can Datagram functions'''

    def test_create_message(self):
        '''Test the constructor for CAN datagrams'''
        message = Datagram(datagram_type_id=TEST_DATAGRAM_TYPE_ID,
                           node_ids=TEST_NODES,
                           data=TEST_DATA)

        self.assertEqual(message._datagram_type_id, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)
        self.assertEqual(message._data, TEST_DATA)

    def test_modify_message(self):
        '''Test the CAN datagram modification'''
        message = Datagram(datagram_type_id=TEST_DATAGRAM_TYPE_ID,
                           node_ids=TEST_NODES,
                           data=TEST_DATA)
        self.assertEqual(message._datagram_type_id, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)
        self.assertEqual(message._data, TEST_DATA)

        message_id = random.randint(0, 10)
        nodes = list(reversed(TEST_NODES))
        dataset = bytearray(reversed(TEST_DATA))

        message._datagram_type_id = message_id
        message._node_ids = nodes
        message._data = dataset

        self.assertEqual(message._datagram_type_id, message_id)
        self.assertEqual(message._node_ids, nodes)
        self.assertEqual(message._data, dataset)

    def test_unpacking(self):
        '''Tests CAN datagram message unpacking'''
        test_bytes = bytearray([1,                          # Datagram Type ID
                                0xE8, 0xBC, 0x16, 0x0F,     # CRC32, little-endian
                                TEST_NODES_RAW1,            # Node ID's RAW
                                TEST_NODES_RAW2,            # Node ID's RAW
                                26, 0,                      # Data Size, little-endian
                                *TEST_DATA                  # Data
                                ])
        message = Datagram._unpack(test_bytes)
        self.assertEqual(message._datagram_type_id, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)
        self.assertEqual(message._data, TEST_DATA)
    
    def test_packing(self):
        message = Datagram(
            datagram_type_id = TEST_DATAGRAM_TYPE_ID,
            node_ids = TEST_NODES,
            data= TEST_DATA
        )

        test_bytes = bytearray([1,
                                0xE8, 0xBC, 0x16, 0x0F,     # CRC32, little-endian
                                TEST_NODES_RAW1,            # Node ID's RAW
                                TEST_NODES_RAW2,            # Node ID's RAW
                                26, 0,                      # Data Size, little-endian
                                *TEST_DATA                  # Data
        ])

        self.assertEqual(message._pack(), test_bytes)

class TestCanDatagramSender(unittest.TestCase):
    '''Test CAN Datagram sending functions'''
    
    def test_send_message(self):
        sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        listener = can.BufferedReader()
        notifier = can.Notifier(sender.bus, [listener])

if __name__ == '__main__':
    unittest.main()
