# pylint: skip-file
"""This Module Tests methods in ping_application.py"""
import random
import time
import unittest

import can
from ping_application import Ping_Application

TEST_CHANNEL = "vcan0"
TEST_DATAGRAM_TYPE_ID = 36  # This is also the start ID


class TestPingApplication(unittest.TestCase):
    """Test the Ping_Application class"""


    def test_unpacking(self):
        """Tests CAN datagram message unpacking
        UNFINISHED"""
        test_bytes = bytearray(
            [
                TEST_NODES_RAW1,  # Node ID's RAW
                TEST_NODES_RAW2,  # Node ID's RAW
                26,
                0,  # Data Size, little-endian
            ]
        )
        message = Datagram._unpack(test_bytes, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)

    def test_ping_application_success(self):
        """Test a successful ping process with valid inputs
        UNFINISHED"""
        node_id = [1, 2, 3]
        data = "TestData"

        try:
            ping_app = Ping_Application(SENDER)
            ping_app.ping_application(node_id=node_id, ping_type="ping", data=data)
        except Exception as e:
            self.fail(f"ping_application raised an exception unexpectedly: {e}")


if __name__ == "__main__":
    unittest.main()
