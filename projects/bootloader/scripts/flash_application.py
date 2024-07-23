from can_datagram import Datagram, DatagramSender
import os
from validation import Validation

CAN_ARBITRATION_FLASH_ID = 32
CAN_ARBITRATION_START_FLASH_ID = 31
TEST_CHANNEL = "vcan0"


class Flash_Application:
    def __init__(self, bin_path, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            # Test channel
            self._sender = DatagramSender()

        if not isinstance(bin_path, str):
            raise ValueError("The bin_path must be a string.")

        if not os.path.isfile(bin_path):
            raise ValueError("The bin_path does not exist.")

        self._bin_path = bin_path
        self._bin_size = os.path.getsize(self._bin_path)

    @property
    def bin_path(self):
        return self._bin_path

    @property
    def bin_size(self):
        return self._bin_size

    @bin_path.setter
    def bin_path(self, value):
        '''This function sets the binary path'''
        if not isinstance(value, str):
            raise ValueError("The bin_path must be a string.")
        if not os.path.isfile(value):
            raise ValueError("The bin_path does not exist.")
        self._bin_path = value
        self._bin_size = os.path.getsize(self._bin_path)

    @bin_size.setter
    def bin_size(self, value):
        '''This function sets the binary size'''
        if not isinstance(value, int) or value < 0:
            raise ValueError("bin_size must be a non-negative integer.")
        self._bin_size = value

    def start_flash(self, **kwargs) -> None:
        node_ids = []
        for val in kwargs["node_ids"]:
            node_ids.append(val & 0xff)

        print(f"Starting flash process")
        print(f'Sending binary data with size of {self._bin_size}...\n\n')

        with open(self._bin_path, 'rb') as bin_data:
            bin_content = bytearray(bin_data.read())

        if len(bin_content) <= 8:
            flash_start_datagram = Datagram(
                datagram_type_id=CAN_ARBITRATION_START_FLASH_ID,
                node_ids=node_ids,
                data=bin_content
            )

            self._sender.send(flash_start_datagram)

        else:
            flash_start_datagram = Datagram(
                datagram_type_id=CAN_ARBITRATION_START_FLASH_ID,
                node_ids=node_ids,
                data=bin_content
            )
            flash_datagram = Datagram(
                datagram_type_id=CAN_ARBITRATION_FLASH_ID,
                node_ids=node_ids,
                data=bin_content[8:]
            )

        self._sender.send(flash_start_datagram, True)
        self._sender.send(flash_datagram, False)

        print(f'\n\nFinished sending flash requirements to boards {node_ids}...')
