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

        # Represents the ping_type, along with branch and project names
        ping_type = kwargs.get("ping_type")
        branch = kwargs.get("branch")
        project = kwargs.get("project")

        meta_datagram = Datagram(
            datagram_type_id=METADATA,
            node_ids=0,
            data=string_to_bytearray(len(branch) if (len(branch)) else len(project))
        )

        print(f"Starting ping application process for boards {node_id}...")

        byte_data = None
        datagram_type = None

        if ping_type == "branch":
            self._sender.send(meta_datagram)
            byte_data = string_to_bytearray(branch)
            datagram_type = BRANCH
        elif ping_type == "project":
            self._sender.send(meta_datagram)
            byte_data = string_to_bytearray(project)
            datagram_type = PROJECT
        elif ping_type == "node_id":
            datagram_type = NODE_ID

        datagram = Datagram(
            datagram_type_id=datagram_type,
            node_ids=node_id if ping_type == "node_id" else 0,
            data=byte_data
        )
        self._sender.send(datagram)
        print(f"Ping application completed for boards {node_id}")

