import unittest
import can
from unittest.mock import patch, mock_open
from flash_application import Flash_Application
from can_datagram import Datagram, DatagramListener, DatagramSender

TEST_CHANNEL = "vcan0"


class TestFlashApplication(unittest.TestCase):
    def setUp(self):
        self.binary_path = '/home/firmware/dev/Akashem06/fwxv/projects/bootloader/test/dummy.bin'
        sender = DatagramSender(bustype="virtual", channel=TEST_CHANNEL, receive_own_messages=True)
        self.app = Flash_Application(bin_path=self.binary_path, sender=sender)

    def test_get_binary_path(self):
        self.assertEqual(
            self.app._bin_path,
            '/home/firmware/dev/Akashem06/fwxv/projects/bootloader/test/dummy.bin')

    def test_get_binary_size(self):
        self.assertEqual(self.app._bin_size, 1725)

    def test_start_flash(self):
        # incomplete
        self.app.start_flash(node_ids=[1, 2])

        # print (self.datagram.node_ids)

        # sent_datagram = DatagramListener(bustype="virtual", channel=TEST_CHANNEL, receive_own_messages=True)
        # self.assertEqual(sent_datagram.datagram_type_id, expected_datagram.datagram_type_id)
        # self.assertEqual(sent_datagram.node_ids, expected_datagram.node_ids)
        # self.assertEqual(sent_datagram.data, expected_datagram.data)


if __name__ == '__main__':
    unittest.main()
