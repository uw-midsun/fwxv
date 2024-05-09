import serial 
import time

ftdi = serial.Serial(port='/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A900NCD9-if00-port0', baudrate=115200, timeout=.1) 

def write_read(x): 
    ftdi.write(bytes(x, 'utf-8')) 
    time.sleep(0.05) 
    data = ftdi.readline() 
    return data 
while True: 
    num = input("") # Taking input from user 
    value = write_read(num) 
    print(value) # printing the value 
