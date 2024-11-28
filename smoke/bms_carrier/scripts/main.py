# import pyvisa
import can
import cantools
import numpy as np
import time
from bms_listener import BMSListener
from discharge_graph import DischargeTestGraph


class HVDischargeTest:
    def __init__(self, instrument, max_pack_voltage, current_amps):
        self.instrument = instrument
        self.instrument.timeout = 5000
        self.instrument.query_delay = 0.1

        self.current_amps = current_amps
        self.max_pack_voltage = max_pack_voltage

        self.pack_voltage = 0
        self.pack_current = current_amps

        self.graph = DischargeTestGraph(self.pack_current)

        self.dbc_file = 'system_can.dbc'
        self.db = cantools.database.load_file(self.dbc_file)
        self.can_bus = can.interface.Bus(channel='vcan0', bustype='virtual')

        self.listener = BMSListener()
        self.listener.db = self.db
        self.listener.can_bus = self.can_bus

    def __del__(self):
        self.stop_test()
        self.graph.stop_plotting()
        self.instrument.close()

    def setup_discharge_test(self):
        self.instrument.write('*RST')
        self.instrument.write('SYSTem:MODE LOAD')
        self.instrument.write(f'LOAD:CURR {self.current_amps}')
        self.instrument.write(f'LOAD:CURR:PROT:HIGH {self.current_amps + 5}')
        # Assuming pack is fully charged
        self.instrument.write(f'LOAD:POW {self.max_pack_voltage * self.current_amps}')
        self.instrument.write(
            f'LOAD:POW:PROT:HIGH {(self.max_pack_voltage + 5) * (self.current_amps + 5)}')

    def measure_values(self):
        instrument_pack_voltage = self.instrument.query('FETCh:VOLTage?')
        instrument_current = self.instrument.query('FETCh:CURRent?')
        instrument_pack_power = self.instrument.query('FETCh:POWer?')

        # Fuse with CAN data from BMS?

    def stop_test(self):
        self.instrument.write('CONFigure:OUTP OFF')
        self.instrument.write('ABORt')

    def run(self):
        # self.instrument.write('CONFigure:OUTPut ON')

        while True:
            if self.pack_voltage <= (4 * 9 * 2.5):
                self.stop_test()
                self.graph.stop_plotting()
                break

            current_time = time.time()
            d_time = current_time - self.last_time
            self.last_time = current_time

            if d_time >= 60:
                self.measure_values()
                self.graph.add_data(self.pack_voltage, current_time)


# rm = pyvisa.ResourceManager('@py')
# print(rm.list_resources())

# instrument = rm.open_resource('ASRL::COM1::INSTR')  # Change to your VISA address

# plotter_1 = DischargeTestGraph(5)
# plotter_2 = DischargeTestGraph(10)

# try:
#     while True:
#         current_time = time.time()
#         voltage = np.random.uniform(3.0, 4.2)

#         plotter_1.add_data(voltage, current_time)
#         plotter_2.add_data(voltage, current_time)

#         if current_time - plotter_1.start_time > 60:
#             break
# finally:
#     plotter_1.stop_plotting()
#     plotter_2.stop_plotting()
