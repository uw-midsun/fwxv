import can
import cantools
import numpy as np
import time
from bms_listener import BMSListener

class TestBMSListener:
    def __init__(self):
        self.dbc_file = '/home/vagrant/shared/fwxv/smoke/bms_carrier/scripts/system_can.dbc'
        self.db = cantools.database.load_file(self.dbc_file)
        self.can_bus = can.interface.Bus(channel='can0', bustype='socketcan')
        
        self.listener = BMSListener()
        self.listener.db = self.db
        self.listener.can_bus = self.can_bus

if __name__ == "__main__":
    test = TestBMSListener()
    while True:
        continue