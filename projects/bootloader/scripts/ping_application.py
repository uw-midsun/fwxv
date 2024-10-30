from bootloader_id import *
from can_datagram import Datagram, DatagramSender

def string_to_bytearray(self, string: str) -> bytearray:
    data = []
    for i in range(len(string)):
        data.append((string[i] >> (i * 8)) & 0xff)
        # [(something[i] >> (i * 8)) & 0xff for i in range(len(something))]
    return bytearray(data)

class Ping_Application:
    def __init__(self, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            self._sender = DatagramSender()

    def ping_application(self, **kwargs):
        node_id = kwargs.get("node_id")

        # Represents the ping_type, along with the data being sent
        ping_type = kwargs.get("ping_type")
        data = kwargs.get("data")

        # Set state to PING
        meta_datagram = Datagram(
            datagram_type_id=PING_METADATA,
            node_ids=0,
            data=string_to_bytearray(len(data))
        )

        print(f"Starting ping application process for boards {node_id}...")

        self._sender.send(meta_datagram)

        # Separate enum that each ECU will read and process accordingly
        datagram = Datagram(
            datagram_type_id=PING, #TO DO fix with proper enum classifying data type
            node_ids=node_id if ping_type == "node_id" else 0,
            data=string_to_bytearray(data)
        )
        self._sender.send(datagram)
        print(f"Ping application completed for boards {node_id}")

