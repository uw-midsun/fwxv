
def read_and_display_hex(file_path):
    with open(file_path, 'rb') as bin_data:
        offset = 0
        while True:
            # Read 16 bytes (128 bits) at a time
            chunk = bin_data.read(16)
            if not chunk:
                break  # End of file

            # Format the output
            hex_output = ' '.join(f"{byte:02x}" for byte in chunk)
            ascii_output = ''.join(
                chr(byte) if 32 <= byte < 127 else '.' for byte in chunk
            )
            
            # Print the offset, hex representation, and ASCII representation
            print(f"{offset:08x}  {hex_output:<48} |{ascii_output}|")
            offset += 16

# Example usage
read_and_display_hex("/home/firmware/dev/Akashem06/fwxv/build/arm/bin/projects/leds.bin")
