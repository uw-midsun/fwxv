from can_datagram import Datagram, DatagramSender

CAN_ARBITRATION_JUMP_ID = 32
TEST_CHANNEL = "vcan0"

class Jump_Application:
    def __init__(self, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            # Test channel
            self._sender = DatagramSender()

    def jump_application(self, **kwargs):
        node_ids =[]
        for val in kwargs["node_ids"]:
            node_ids.append(val & 0xff)
        
        print(f"Starting jump application process for boards {node_ids}...")
        
        jump_datagram = Datagram(
            datagram_type_id = CAN_ARBITRATION_JUMP_ID,
            node_ids = node_ids,
            data = bytearray(),
        )
        self._sender.send(jump_datagram)
        print(f"Jump application completed for boards {node_ids}")
