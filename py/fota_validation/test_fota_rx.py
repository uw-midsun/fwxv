import serial
import time

def receive_data(ser):
    try:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            print(f"Receiving data: {data}")  # Not as hex
        else:
            print("No data available")
    except Exception as e:
        print(f"Error while receiving data: {e}")

def main():
    PORT = "/dev/ttyUSB0"
    BAUDRATE = 115200
    TIMEOUT = 1

    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=TIMEOUT)
        print(f"\nConnected to port {PORT} at {BAUDRATE} baud!\n")

        while True:
            receive_data(ser)
            time.sleep(1)

    except Exception as e:
        print(f"Error: {e}")
    finally:
        if ser.is_open:
            ser.close()
            print(f"\nConnection to port {PORT} closed!\n")

if __name__ == "__main__":
    main()