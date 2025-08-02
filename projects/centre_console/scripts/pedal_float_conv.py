"""
Python file to convert uint32_t pedal messages into floats between 1-0
"""
import struct


def prv_get_float_py(u_int32: int) -> float:
    """
    Converts uint32_t pedal reading into a float between 1.0 and 0.0
    """
    packed_bytes = struct.pack('>I', u_int32)
    reinterpreted_float = struct.unpack('>f', packed_bytes)[0]

    return reinterpreted_float


UINT_VAL_TEST_VAL = 1065353216
float_val = prv_get_float_py(UINT_VAL_TEST_VAL)
print(f"Reinterpreting 0x{UINT_VAL_TEST_VAL:08X} (as uint32): {float_val}")
