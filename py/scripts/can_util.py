"""This module implements methods to send and receive sample CAN data for testing other modules."""

from collections import namedtuple

import can

DEVICE_ID = 15
MESSAGE_ID = 63

# Set vcan0 as the default. Make sure to have virtual CAN set up in this case.
default_channel = "can0"  # pylint: disable=invalid-name

# We use the standard 500kbps baudrate.
CAN_BITRATE = 500000

# A mapping for caching data about each channel, and a handy type used for the cached data.
# We use a cache so that there's only one notifier thread running per channel.
channels_to_data = {}
ChannelData = namedtuple("ChannelData", ["bus", "reader", "notifier"])


def get_bus_data(channel=None):
    """Returns the memoized ChannelData for a given channel"""

    if channel is None:
        channel = default_channel

    if channel not in channels_to_data:
        bus = can.interface.Bus(bustype="socketcan", channel=channel, bitrate=CAN_BITRATE)

        # We use a BufferedReader rather than reading straight from the Bus to prevent the scenario
        # where the time for the second message to be tx'd exceeds the time taken to process the
        # first message and start listening for the second one, which occurs if the C side is fast
        # enough. By buffering, we never miss a message.
        reader = can.BufferedReader()
        notifier = can.Notifier(bus, [reader])

        channels_to_data[channel] = ChannelData(bus, reader, notifier)

    return channels_to_data[channel]


def get_bus(channel=None):
    """Returns a Python-CAN Bus for sending messages."""
    return get_bus_data(channel).bus


class Message:
    """
    An immutable wrapper over Python-CAN's can.Message to support our message and device ID
    conventions. See https://python-can.readthedocs.io/en/master/message.html.

    Attributes:
        msg: The underlying can.Message.
        message_id: The message ID of the CAN message.
        device_id: The device ID of the CAN message.
        data: The data associated with the message.
    """

    def __init__(self, message_id=0, device_id=0, **kwargs):
        """Initialize a Message. See Python-CAN's can.Message for more info.

        Args:
            message_id: The message ID of the CAN message, used if arbitration_id is not passed.
            device_id: The device ID of the CAN message, used if arbitration_id is not passed.
        """

        if "arbitration_id" not in kwargs:
            # our CAN system uses 6 bits of message ID, 1 bit for ACK/DATA, and 4 bits for device ID
            kwargs["arbitration_id"] = (message_id << 5) | device_id

        self.msg = can.Message(**kwargs)

    @classmethod
    def from_msg(cls, msg):
        """Helper to get a Message from a can.Message."""
        message = cls()
        message.msg = msg
        return message

    @property
    def message_id(self):
        """The message ID of this CAN message."""
        # message ID is bits 5-11
        return (self.msg.arbitration_id >> 5) & 0b111111

    @property
    def device_id(self):
        """The device ID of this CAN message."""
        # device ID is bits 0-3
        return self.msg.arbitration_id & 0b1111

    @property
    def data(self):
        """The data associated with this CAN message."""
        return self.msg.data


def send_message(
        id=None,
        data=None,
        channel=None,
        msg_id=MESSAGE_ID,
        device_id=DEVICE_ID,
):
    """Sends a CAN message.

    Args:
        id: The ID (first byte of message data) of the message to send. If
            None, the first byte of message data isn't overwritten.
        data: The data to send in the CAN message. Must be a list of bytes (0-255). If id
            is None, this can be up to 8 bytes; otherwise, it can only be up to 7 bytes since the
            first byte is the ID.
        channel: The SocketCAN channel on which to send the message.
        msg_id: The CAN message ID to use.
        device_id: The device ID to use.

    Raises:
        can.CanError: If there was an error in transmitting the message.
    """

    if data is None:
        data = []
    if id is not None:
        data = [id] + data

    if len(data) > 8:
        raise ValueError("Only 8 bytes of data (including ID) may be sent")
    if len(data) < 8 and msg_id == MESSAGE_ID:
        # pad to 8 bytes so that the firmware project will accept it
        data += [0] * (8 - len(data))

    data = bytearray(data)

    bus = get_bus(channel)

    msg = Message(
        message_id=msg_id,
        device_id=device_id,
        data=data,
        is_extended_id=False
    )

    bus.send(msg.msg)
