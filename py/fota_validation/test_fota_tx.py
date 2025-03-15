import serial

def send_data(ser, data):
    try:
        data_in_bytes = bytes.fromhex(data)
        print(f"Sending user data: {data_in_bytes}")
        ser.write(data_in_bytes)
    except ValueError:
        print("Invalid input. Please try entering a proper hexidecimal value")

def main():
    PORT = "/dev/ttyUSB2"
    BAUDRATE = 115200
    TIMEOUT = 1

    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=TIMEOUT)
        print(f"\nConnected to port {PORT} at {BAUDRATE} baud!\n")

        # Test transmission during startup
        sample_data = b'\xAA\x00\x00\x00\x01\x07\x01\x01\x01\x01\x01\x01\x01\xBB'

        print(f"Sending sample data: {sample_data}")
        ser.write(sample_data)

        while True:
            user_input = input("\nEnter hexidecimal data (or 'exit' to quit): ")

            if user_input.lower() == "exit":
                break

            send_data(ser, user_input)

    except Exception as e:
        print(f"Error: {e}")

    finally:
        if ser.is_open:
            ser.close()
            print(f"\nConnection to port {PORT} closed!\n")

if __name__ == "__main__":
    main()