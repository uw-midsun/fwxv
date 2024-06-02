from can_datagram import Datagram, DatagramSender
import os
from validation import Validation

CAN_ARBITRATION_FLASH_ID = 0b00000000001

class Flash_Application():
    def __init__(self, binary_path, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            self._sender = DatagramSender()
        
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
    
    def start_initial_process(self, board_nums: int) -> None:
        print(f"Starting the initial flash process")

        print(f'Sending binary data with size of {self.get_binary_size()}...')
        
        board_ids = list(range(board_nums + 1))

        if self.validate_bin() and Validation.validate_board_id(board_nums):
            # Create a datagram with the initial flash message
            for board_id in board_ids:
                initial_datagram = Datagram(
                    datagram_type_id=CAN_ARBITRATION_FLASH_ID | (board_id << 5),
                    node_ids=board_ids,
                    data=bytearray([self.get_binary_size() & 0xff, (self.get_binary_size() >> 8) & 0xff]) 
                )

                self._sender.send(initial_datagram)

                print(f'Send {self._bin_path} to board {board_id}')
            
            print(f'Finished initial flash requirements to board {board_ids}...')

    def stream_flash_data(self, board_nums: int):
        board_ids = list(range(board_nums + 1))
        
        print(f'Starting streaming of boards {board_ids}')
        
        if self.validate_bin() and Validation.validate_board_id(board_nums):
            for board_id in board_ids:
                with open(self.get_binary_path(), 'rb') as bin_data:
                        bin_content = bytearray(bin_data.read())

                        flash_datagram = Datagram(
                            datagram_type_id=CAN_ARBITRATION_FLASH_ID | (board_id << 5),
                            node_ids=board_ids,
                            data=bin_content
                        )

                        self._sender.send(flash_datagram)

                    

                    


        print(f"Streaming of flash data completed for boards {board_ids}")
