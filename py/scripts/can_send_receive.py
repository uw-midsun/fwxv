"""Sends user-specified CAN messages, without needing to first send them through the STM32."""
"""This module is mainly targeted towards testing FSMs."""

import cantools
import can_util
import time

# global var for the Database from load_dbc
# pylint: disable=invalid-name
dbc_database = None
DEFAULT_ID = 15
DEFAULT_TIMEOUT = 10

def load_dbc(dbc_filename):
    """
    Creates a Database object from an existing DBC file used to encode CAN messages.
    Args:
        dbc_filename: a string representing the path to a DBC file
    """
    # pylint: disable=global-statement
    global dbc_database
    dbc_database = cantools.database.load_file(dbc_filename)


def can_send_raw(msg_id, data, DEFAULT_ID=DEFAULT_ID, channel=None):
    """
    A wrapper over can_utils.send_message providing a friendlier interface to can_util.send_message.
    Args:
        data: a list of up to 8 bytes of data to send
        channel: CAN channel
        msg_id: CAN message ID
        DEFAULT_ID: the device ID to send from
    """
    can_util.send_message(
        data=data,
        channel=channel,
        msg_id=msg_id,
        DEFAULT_ID=DEFAULT_ID,
    )

# Not sure if we need this, but might come in handy if we are testing with DBC files
def can_send(msg_name, channel=None, **data):
    """
    Uses the Database object created by the load_dbc function to encode a CAN message
    Args:
        msg_name: the string name of the message
        channel: CAN channel
        data: a dictionary of field names (strings) to values (integers)
    """
    msg_obj = dbc_database.get_message_by_name(msg_name.upper())
    msg_obj_frame_id = msg_obj.frame_id
    msg_obj_data = msg_obj.encode(data)

    bus = can_util.get_bus(channel)
    can_msg = can_util.Message(arbitration_id=msg_obj_frame_id, data=msg_obj_data)
    bus.send(can_msg.msg)


def can_receive(channel=None):
    """
    Receive messages sent to the can Bus
    """
    start_time = time.time()

    if channel == None:
        channel = can_util.default_channel

    bus = can_util.get_bus(channel)

    recv_msg_list = []
    # Receive the data. We store the data in a buffered reader, but we do not want to pick up data that we sent ourselves.     
    while DEFAULT_TIMEOUT - time.time() > 0:
        recv_msg = channel.notifier.get_message()
        
        if recv_msg.is_rx:
            recv_msg_list.append(recv_msg)

    return recv_msg_list

# Pass in a list of arbitrary size of FSM string data chunks into the function to send and listen to any responses sent back on the same channel.
def can_trigger_fsm_transitions(msg_id, default_id=DEFAULT_ID, channel=None, *args):
    
    for arg in args:
        can_send_raw(msg_id, arg, default_id, channel)

    can_receive(channel)
    return True