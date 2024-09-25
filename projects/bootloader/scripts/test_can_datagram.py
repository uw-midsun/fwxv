# pylint: skip-file
'''This Module Tests methods in can_datagram.py'''
import unittest
import random
import time

from can_datagram import Datagram, DatagramSender, DatagramListener
import can

TEST_CHANNEL = "vcan0"

TEST_DATAGRAM_TYPE_ID = 30  # This is also the start ID
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

        self.assertEqual(message._node_ids, TEST_NODES)
        self.assertEqual(message._data, TEST_DATA)

    def test_modify_message(self):
        '''Test the CAN datagram modification'''
        message = Datagram(datagram_type_id=TEST_DATAGRAM_TYPE_ID,
                           node_ids=TEST_NODES,
                           data=TEST_DATA)
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
        test_bytes = bytearray([TEST_NODES_RAW1,            # Node ID's RAW
                                TEST_NODES_RAW2,            # Node ID's RAW
                                26, 0,                      # Data Size, little-endian
                                ])
        message = Datagram._unpack(test_bytes, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)

    def test_packing(self):
        message = Datagram(
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=TEST_DATA
        )

        test_bytes = bytearray([TEST_NODES_RAW1,            # Node ID's RAW
                                TEST_NODES_RAW2,            # Node ID's RAW
                                ])

        self.assertEqual(message.pack(), test_bytes)


class TestCanDatagramSender(unittest.TestCase):
    '''Test CAN Datagram sending functions'''

    def test_send_message(self):
        sender = DatagramSender(bustype="virtual", channel=TEST_CHANNEL, receive_own_messages=True)
        listener = can.BufferedReader()
        notifier = can.Notifier(sender.bus, [listener])

        message = Datagram(
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))

        sender.send(message, True)

        recv_datagram = []
        listener_message = listener.get_message()

        while listener_message is not None:
            for byte in listener_message.data:
                recv_datagram.append(byte)
            listener_message = listener.get_message()

        # self.assertEqual(message.pack(), bytearray(recv_datagram))


class TestCanDatagramListener(unittest.TestCase):
    '''Test CAN Datagram listening functions'''

    def test_register_callback(self):
        '''Test the registering of a callback'''
        self.callback_triggered = False

        sender = DatagramSender(bustype="virtual", channel=TEST_CHANNEL, receive_own_messages=True)
        listener = DatagramListener(self.triggerCallback)
        notifier = can.Notifier(sender.bus, [listener])

        message = Datagram(
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))
        sender.send(message, True)

        timeout = time.time() + 10
        while not self.callback_triggered:
            if time.time() > timeout:
                break

        #self.assertEqual(self.message, message.pack())
        self.assertEqual(self.callback_triggered, True)

    def triggerCallback(self, msg, board_id):
        self.message = msg
        self.callback_triggered = True


if __name__ == '__main__':
    unittest.main()
