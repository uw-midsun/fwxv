import unittest
from unittest.mock import MagicMock, patch
from jump_application import Jump_Application, CAN_ARBITRATION_JUMP_ID
from can_datagram import Datagram, DatagramSender

TEST_CHANNEL = "vcan0"


class TestJumpApplication(unittest.TestCase):

    def setUp(self):
        sender = DatagramSender(bustype="virtual", channel=TEST_CHANNEL, receive_own_messages=True)
        self.app = Jump_Application(sender=sender)

    def test_start_jump_process_valid(self):
        test_node_ids = [1, 2]
        self.app.jump_application(node_ids=test_node_ids)

        # self.mock_sender.send.assert_called_once()

        # # Extract the actual datagram sent
        # actual_datagram = self.mock_sender.send.call_args[0][0]

        # # Compare the attributes
        # self.assertEqual(actual_datagram.datagram_type_id, expected_datagram.datagram_type_id)
        # self.assertEqual(actual_datagram.node_ids, expected_datagram.node_ids)
        # self.assertEqual(actual_datagram.data, expected_datagram.data)


if __name__ == '__main__':
    unittest.main()
