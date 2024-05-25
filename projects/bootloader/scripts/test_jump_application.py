import unittest
from unittest.mock import MagicMock, patch
from jump_application import Jump_Application, CAN_ARBITRATION_JUMP_ID
from can_datagram import Datagram

class TestJumpApplication(unittest.TestCase):

    @patch('jump_application.DatagramSender', autospec=True)
    def setUp(self, MockSender):
        self.mock_sender = MockSender.return_value
        self.app = Jump_Application(sender=self.mock_sender)

    def test_validate_board_id_valid(self):
        self.assertTrue(self.app.validate_board_id(1))

    def test_validate_board_id_invalid(self):
        self.assertFalse(self.app.validate_board_id(-1))
        self.assertFalse(self.app.validate_board_id("string"))

    def test_start_jump_process_valid(self):
        board_id = 1
        self.app.start_jump_process(board_id)
        expected_datagram = Datagram(
            datagram_type_id=CAN_ARBITRATION_JUMP_ID | (board_id << 5),
            node_ids=[board_id],
            data=bytearray()
        )
        self.mock_sender.send.assert_called_once_with(expected_datagram)

    def test_start_jump_process_invalid(self):
        board_id = -1
        self.app.start_jump_process(board_id)
        self.mock_sender.send.assert_not_called()

if __name__ == '__main__':
    unittest.main()
