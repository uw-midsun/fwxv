from can_datagram import Datagram, DatagramSender
import os

CAN_ARBITRATION_FLASH_ID = 0b00000000001

class Flash_Application():
    def __init__(self, binary_path, sender=DatagramSender()) -> None:
        self._sender = sender
        self._bin_path = binary_path
        self._bin_size = os.path.getsize(self._bin_path)


    def get_binary_path(self):
        return self._bin_path
    
    def get_binary_size(self):
        return self._bin_size
    
    def validate_bin(self):
        if os.path.isfile(self.get_binary_path()):
            return True
        
        print(f"Could not find the binary file for flashing...")
        return False
    
    def validate_board_id(self, board_int: int):
        if isinstance(board_int, int) and board_int >= 0:
            return True
        
        print(f'Invalid Board ID: {board_int}')
        return False
    
    def start_initial_process(self, board_id: int) -> None:
        print(f"Starting the initial flash process")

        print(f'Sending binary data with size of {self.get_binary_size()}...')

        if self.validate_bin() and self.validate_board_id(board_id):
            # Create a datagram with the initial flash message
            initial_datagram = Datagram(
                datagram_type_id=CAN_ARBITRATION_FLASH_ID | (board_id << 5),
                node_ids=[board_id],
                data=bytearray([self.get_binary_size() & 0xff, (self.get_binary_path() >> 8) & 0xff]) 
            )

            self._sender.send(initial_datagram)
            
            print(f'Finished initial flash requirements to board {board_id}...')

    def stream_flash_data(self, board_id: int):
        print(f"Streaming flash data to board {board_id}...")

        if self.validate_bin and self.validate_board_id(board_id):
            with open(self.get_binary_path(), 'rb') as bin_data:
                flash_datagram = Datagram(
                    datagram_type_id=CAN_ARBITRATION_FLASH_ID | (board_id << 5),
                    node_ids=[board_id],
                    data=bytearray(bin_data.read())
                )

                self._sender.send(flash_datagram)


        print(f"Streaming of flash data completed for board {board_id}")
