import unittest
import can
from unittest.mock import patch, mock_open
from flash_application import Flash_Application, CAN_ARBITRATION_FLASH_ID
from can_datagram import Datagram, DatagramListener, DatagramSender

TEST_CHANNEL = "vcan0"

class TestFlashApplication(unittest.TestCase):
    def setUp(self):
        self.binary_path = '/home/vagrant/shared/fwxv/projects/bootloader/test/dummy.bin'
        sender = DatagramSender(bustype="virtual", channel=TEST_CHANNEL, receive_own_messages=True)
        self.app = Flash_Application(bin_path=self.binary_path, sender=sender)

    def test_get_binary_path(self):
        self.assertEqual(self.app._bin_path, '/home/vagrant/shared/fwxv/projects/bootloader/test/dummy.bin')

    def test_get_binary_size(self):
        self.assertEqual(self.app._bin_size, 37)

    def test_start_flash(self):
        #incomplete
        listener = DatagramListener(self.triggerCallback)
        notifier = can.Notifier(self.app._sender.bus, [listener])
        self.app.start_flash(node_ids=[1,2])


        expected_datagram = Datagram(
            datagram_type_id=CAN_ARBITRATION_FLASH_ID,
            node_ids=[1, 2],
            data=bytearray([37 & 0xff, (37 >> 8) & 0xff])
        )
        # print (self.datagram.node_ids)

        # sent_datagram = DatagramListener(bustype="virtual", channel=TEST_CHANNEL, receive_own_messages=True)
        # self.assertEqual(sent_datagram.datagram_type_id, expected_datagram.datagram_type_id)
        # self.assertEqual(sent_datagram.node_ids, expected_datagram.node_ids)
        # self.assertEqual(sent_datagram.data, expected_datagram.data)

    def triggerCallback(self, msg, board_id):
        self.message = msg

if __name__ == '__main__':
    unittest.main()
