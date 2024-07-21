
from can_datagram import Datagram, DatagramSender

CAN_ARBITRATION_PING_ID = 0b0000000100


class Validation:
    def __init__(self, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            self._sender = DatagramSender()

    @staticmethod
    def validate_board_id(board_int: int):
        if isinstance(board_int, int) and board_int >= 0 and board_int < 16:
            return True

        print(f'Invalid Board ID: {board_int}')
        return False

    @staticmethod
    def validate_jump_size(application_int: int):
        if isinstance(application_int, int) and application_int >= 0 and application_int < pow(2, 16):
            return True

        print(f'Jump Application command is not valid for {application_int}')
        return False

    # TODO Add Ping command to validate if board recieves messages
    def recieve_message():
        pass

    def ping_board(self, board_int: int):
        print(f'Initiated to ping board {board_int} to validate active state')

        if self.validate_board_id(board_int):
            ping_datagram = Datagram(
                datagram_type_id=CAN_ARBITRATION_PING_ID | (board_int << 5),
                node_ids=[board_int],
                data=bytearray()
            )

            self._sender.send(ping_datagram)
