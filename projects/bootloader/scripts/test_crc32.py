from crc32 import CRC32

data = b'Test data for CRC32'
crc = CRC32(0x04C11DB7)  # Standard CRC32 polynomial
result = crc.calculate(data)
print(f"CRC32 (Python): {result}")