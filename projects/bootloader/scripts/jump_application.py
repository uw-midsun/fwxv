from can_datagram import Datagram, DatagramSender

CAN_ARBITRATION_JUMP_ID = 0b00000000001

class Jump_Application:
    def __init__(self, sender: DatagramSender) -> None:
        self.sender = sender

    def jump_application_process(self, datagram: Datagram, board_id: int, flash_data):
        print(f"Starting jump application process for board {board_id}...")
        
        #TODO Perform actual flash application logic here
        message_id = CAN_ARBITRATION_JUMP_ID | board_id << 5

        datagram.datagram_type_id = message_id
        datagram.node_ids = [board_id]

        self.sender.send(bytearray(flash_data))

        print(f"Jump application completed for board {board_id}")

    def start_jump_process(self):
        datagram = Datagram()

        # self.jump_application_process(datagram, )
