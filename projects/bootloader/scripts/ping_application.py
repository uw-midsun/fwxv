from bootloader_id import *
from can_datagram import Datagram, DatagramSender


def string_to_bytearray(self, string: str) -> bytearray:
    data = []
    for i in range(len(string)):
        data.append((string[i] >> (i * 8)) & 0xFF)
        # [(something[i] >> (i * 8)) & 0xff for i in range(len(something))]
    return bytearray(data)


class Ping_Application:
    def __init__(self, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            self._sender = DatagramSender()

    def ping_application(self, **kwargs):
        start_time = time.time()
        # node_id = kwargs.get("node_id")

        # Represents the ping_type, along with the data being sent
        ping_type = kwargs.get("ping_type")

        data = kwargs.get("data")

        # Set state to PING
        meta_datagram = Datagram(
            datagram_type_id=PING_METADATA,
            request=0,  # this represents the type of request
            node_ids=0,
            data=string_to_bytearray(len(data)),
        )

        print(f"Starting ping application process for boards {node_id}...")

        self._sender.send(meta_datagram)

        # Start chunking the data and sending it
        chunk_messages = list(self._chunkify(string_to_bytearray(data), 8))
        sequence_number = 0

        while chunk_messages:
            # Prepare up to 1024 bytes (128 chunks of 8 bytes each)
            current_chunk = chunk_messages[:128]
            chunk_messages = chunk_messages[128:]

            crc_chunk = b"".join(current_chunk)
            crc32_value = crc32.calculate(crc_chunk)
            crc_data = crc32_value.to_bytes(4, byteorder="little")

            # Send data chunks (up to 1024 bytes)
            for chunk in current_chunk:
                try:
                    data_msg = can.Message(
                        arbitration_id=PING_DATA,
                        data=chunk,
                        is_extended_id=message_extended_arbitration,
                    )
                    self.bus.send(data_msg)
                except BaseException:
                    time.sleep(0.01)
                    self.bus.send(data_msg)

            print(f"Sent {len(current_chunk) * 8} bytes")

            if chunk_messages:
                ack_received = False
                retry_count = 0
                max_retries = 3

                while not ack_received and retry_count < max_retries:
                    try:
                        ack_msg = self.bus.recv(timeout=5.0)

                        if ack_msg and ack_msg.arbitration_id == ACK:
                            if ack_msg.data[0] == 0x01:
                                ack_received = True
                                print(f"Received ACK for sequence {sequence_number}\n")
                            elif ack_msg.data[0] == 0x00:
                                print(
                                    f"Received NACK for sequence {sequence_number}, retrying..."
                                )
                                retry_count += 1
                                break
                            else:
                                print(
                                    f"Received unknown response for sequence {sequence_number}, retrying..."
                                )
                                retry_count += 1
                        else:
                            print(
                                f"No ACK/NACK received for sequence {sequence_number}, retrying..."
                            )
                            retry_count += 1

                    except can.CanError:
                        print(
                            f"Error waiting for ACK/NACK for sequence {sequence_number}, retrying..."
                        )
                        retry_count += 1

                if not ack_received:
                    raise Exception(
                        f"Failed to receive ACK for sequence {sequence_number} after {max_retries} attempts"
                    )

        end_time = time.time()

        print(
            "--------------------------------- COMPLETED ---------------------------------"
        )
        print(f"Time Elapsed: {end_time - start_time}")
        print(f"All data sent successfully. Total sequences: {sequence_number}\n")

        ### End
        # Separate enum that each ECU will read and process accordingly
        # datagram = Datagram(datagram_type_id=PING_DATA, data=string_to_bytearray(data))
        # self._sender.send(datagram)
        print(f"Ping application completed for boards {node_id}")
