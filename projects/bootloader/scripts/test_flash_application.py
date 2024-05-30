import unittest
from unittest.mock import patch, mock_open
from flash_application import Flash_Application, CAN_ARBITRATION_FLASH_ID
from can_datagram import Datagram

class TestFlashApplication(unittest.TestCase):
    
    @patch('flash_application.DatagramSender', autospec=True)
    @patch('os.path.getsize', return_value=1024)
    @patch('os.path.isfile', return_value=True)
    def setUp(self, mock_isfile, mock_getsize, MockSender):
        self.mock_sender = MockSender.return_value
        self.binary_path = 'dummy_path.bin'
        self.app = Flash_Application(binary_path=self.binary_path, sender=self.mock_sender)

    def test_get_binary_path(self):
        self.assertEqual(self.app.get_binary_path(), 'dummy_path.bin')

    def test_get_binary_size(self):
        self.assertEqual(self.app.get_binary_size(), 1024)

    @patch('os.path.isfile', return_value=True)
    def test_validate_bin_valid(self, mock_isfile):
        self.assertTrue(self.app.validate_bin())

    @patch('os.path.isfile', return_value=False)
    def test_validate_bin_invalid(self, mock_isfile):
        self.assertFalse(self.app.validate_bin())

    @patch('os.path.isfile', return_value=True)
    @patch('builtins.open', new_callable=mock_open, read_data=b'testdata')
    def test_start_initial_process(self, mock_open, mock_isfile):
        board_id = 1
        self.app.start_initial_process(board_id)
        expected_datagram = Datagram(
            datagram_type_id=CAN_ARBITRATION_FLASH_ID | (board_id << 5),
            node_ids=[board_id],
            data=bytearray([1024 & 0xff, (1024 >> 8) & 0xff])
        )
        # Comparing attributes instead of objects
        self.assertEqual(self.mock_sender.send.call_args[0][0].datagram_type_id, expected_datagram.datagram_type_id)
        self.assertEqual(self.mock_sender.send.call_args[0][0].node_ids, expected_datagram.node_ids)
        self.assertEqual(self.mock_sender.send.call_args[0][0].data, expected_datagram.data)

    @patch('os.path.isfile', return_value=True)
    @patch('builtins.open', new_callable=mock_open, read_data=b'\x01\x02\x03\x04\x05\x06\x07\x08\x09')
    def test_stream_flash_data(self, mock_open, mock_isfile):
        board_id = 1
        self.app.stream_flash_data(board_id)
        expected_data = bytearray(b'\x01\x02\x03\x04\x05\x06\x07\x08\x09')
        expected_datagram = Datagram(
            datagram_type_id=CAN_ARBITRATION_FLASH_ID | (board_id << 5),
            node_ids=[board_id],
            data=expected_data
        )
        # Comparing attributes instead of objects
        self.assertEqual(self.mock_sender.send.call_args[0][0].datagram_type_id, expected_datagram.datagram_type_id)
        self.assertEqual(self.mock_sender.send.call_args[0][0].node_ids, expected_datagram.node_ids)
        self.assertEqual(self.mock_sender.send.call_args[0][0].data, expected_datagram.data)

if __name__ == '__main__':
    unittest.main()
