from bootloader import *

class Ping_Application:
    def __init__(self, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            self._sender = DatagramSender()

    def string_to_bytearray(self, string: str) -> bytearray:
        data = []
        for i in range(len(string)):
            data.append((string[i] >> (i * 8)) & 0xff)
            # [(something[i] >> (i * 8)) & 0xff for i in range(len(something))]
        return bytearray(data)

    def ping_application(self, **kwargs):
        node_id = kwargs["node_id"]

        # Represents the ping_type, along with branch and project names
        ping_type = kwargs["ping_type"]
        branch = kwargs["branch"]
        project = kwargs["project"]

        print(f"Starting ping application process for boards {node_ids}...")

        byte_data = None
        datagram_type = None

        if ping_type == "branch":
            byte_data = string_to_bytearray(branch)
            datagram_type = bootloader_id.BRANCH
        elif ping_type == "project":
            byte_data = string_to_bytearray(project)
            datagram_type = bootloader_id.PROJECT
        elif ping_type == "node_id":
            datagram_type = bootloader_id.NODE_ID

        datagram = Datagram(
            datagram_type_id=datagram_type,
            node_ids=node_id if ping_type == "node_id" else 0,
            data=byte_data
        )
        self._sender.send(datagram)
        print(f"Ping application completed for boards {node_ids}")

