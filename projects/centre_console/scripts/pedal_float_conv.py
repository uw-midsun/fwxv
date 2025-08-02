import struct

def prv_get_float_py(u_int32: int) -> float:
    packed_bytes = struct.pack('>I', u_int32)
    reinterpreted_float = struct.unpack('>f', packed_bytes)[0]

    return reinterpreted_float

uint_val_test_val_0 = 1065353216
float_val_1_0 = prv_get_float_py(uint_val_test_val_0)
print(f"Reinterpreting 0x{uint_val_test_val_0:08X} (as uint32): {float_val_1_0}")
