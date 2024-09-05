import pyvisa

rm = pyvisa.ResourceManager('@py')
print(rm.list_resources())

# instrument = rm.open_resource('ASRL::COM1::INSTR')  # Change to your VISA address

# instrument.timeout = 5000 # Milliseconds
# instrument.query_delay = 0.1

# def setup_discharge_test(current_amps, pack_voltage):
#     """Sets up and starts a discharge test with a specific current"""

#     instrument.write('*RST')

#     instrument.write(f'SOUR: CURR {current_amps}')    
#     instrument.write(f'SOUR: VOLT {pack_voltage}')
    
#     # Enable the output
#     instrument.write('CONFigure:OUTPut ON')

# def measure_values():
#     """Reads back the current and voltage"""

#     current = instrument.query(':MEAS:CURR?')

#     voltage = instrument.query(':MEAS:VOLT?')

#     power = instrument.query('MEAS:POWer?')
    
#     return float(current), float(voltage)

# def stop_test():
#     """Stops the test and turns off the output"""

#     instrument.write(':OUTP OFF')


# if __name__ == '__main__':
#     setup_discharge_test(6, 12)

#     start_time = time.time()
#     while time.time() - start_time < 10:
#         current, voltage = measure_values()
#         print(f"Current: {current} A, Voltage: {voltage} V")
#         time.sleep(1)  # Measure every 1 second

#     stop_test()

#     instrument.close()