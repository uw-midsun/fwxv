from can_datagram import Datagram, DatagramSender

CAN_ARBITRATION_JUMP_ID = 0b00000000011

class Jump_Application:
    def __init__(self, sender: DatagramSender) -> None:
        self._sender = sender

    def validate_board_id(self, board_int: int):
        if isinstance(board_int, int) and board_int >= 0:
            return True
        
        print(f'Invalid Board ID: {board_int}')
        return False

    def start_jump_process(self, board_id: int) -> None:
        if not self.validate_board_id(board_id):
            return

        print(f"Starting jump process for board {board_id}...")

        datagram_id = CAN_ARBITRATION_JUMP_ID | (board_id << 5)

        # Create a datagram with the jump message
        jump_datagram = Datagram(
            datagram_type_id=datagram_id,
            node_ids=[board_id],
            data=bytearray()
        )

<<<<<<< HEAD
        # self.jump_application_process(datagram, )
=======
        self._sender.send(jump_datagram)
        
        print(f"Jump process initiated for board {board_id}...")
>>>>>>> add jump application, both flash and jump app needs testing
