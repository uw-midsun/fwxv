import unittest
from unittest.mock import MagicMock, patch
from jump_application import Jump_Application, CAN_ARBITRATION_JUMP_ID
from can_datagram import Datagram

class TestJumpApplication(unittest.TestCase):

    @patch('jump_application.DatagramSender', autospec=True)
    def setUp(self, MockSender):
        self.mock_sender = MockSender.return_value
        self.app = Jump_Application(sender=self.mock_sender)

    def test_start_jump_process_valid(self):
        board_id = 1
        jump_id = 1
        
        self.app.start_jump_process(16, pow(2, 16))
        self.app.start_jump_process(-1, -1)
        self.app.start_jump_process(board_id, jump_id)
        datagram_id = CAN_ARBITRATION_JUMP_ID | (board_id << 5)
        expected_datagram = Datagram(
            datagram_type_id=datagram_id,
            node_ids=[0, 1],
            data=bytearray(jump_id)
        )

        self.mock_sender.send.assert_called_once()

        # Extract the actual datagram sent
        actual_datagram = self.mock_sender.send.call_args[0][0]

        # Compare the attributes
        self.assertEqual(actual_datagram.datagram_type_id, expected_datagram.datagram_type_id)
        self.assertEqual(actual_datagram.node_ids, expected_datagram.node_ids)
        self.assertEqual(actual_datagram.data, expected_datagram.data)

if __name__ == '__main__':
    unittest.main()
