class CRC32:
    crc_table = {}

    def __init__(self, _poly):
        for i in range(256):
            c = i << 24
            for j in range(8):
                c = (c << 1) ^ _poly if (c & 0x80000000) else c << 1
            self.crc_table[i] = c & 0xFFFFFFFF

    def calculate(self, buf):
        crc = 0xFFFFFFFF
        buf_len = len(buf)
        i = 0

        while i + 4 <= buf_len:
            b = [buf[i+3], buf[i+2], buf[i+1], buf[i+0]]  
            i += 4
            for byte in b:
                crc = ((crc << 8) & 0xFFFFFFFF) ^ self.crc_table[(crc >> 24) ^ byte]

        if i < buf_len:
            remaining = buf[i:buf_len] + b'\x00' * (4 - (buf_len - i))
            b = [remaining[3], remaining[2], remaining[1], remaining[0]]
            for byte in b:
                crc = ((crc << 8) & 0xFFFFFFFF) ^ self.crc_table[(crc >> 24) ^ byte]

        return crc ^ 0xFFFFFFFF

    def crc_int_to_bytes(self, i):
        return [(i >> 24) & 0xFF, (i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF]
