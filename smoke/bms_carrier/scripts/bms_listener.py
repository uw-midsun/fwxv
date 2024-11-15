import can
import cantools
import numpy as np
import matplotlib.pyplot as plt
import time
import csv
from datetime import datetime

class BMSListener:
    def __init__(self):
        # CAN setup
        self.db = cantools.database.load_file('/home/vagrant/shared/fwxv/smoke/bms_carrier/scripts/system_can.dbc')
        self.can_bus = can.interface.Bus(channel='can0', bustype='socketcan')

        self.start_time = time.time()

        self.cell_voltages_1 = []
        self.cell_voltages_2 = []
        self.cell_voltages_3 = []

        self.temperatures_1 = []
        self.temperatures_2 = []
        self.temperatures_3 = []

        self.afe1_sampled = False
        self.afe2_sampled = False
        self.afe3_sampled = False

        self.last_id = 0

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.csv_battvt = f"/home/vagrant/shared/fwxv/smoke/bms_carrier/csv_data/BATTVT_{timestamp}.csv"
        with open(self.csv_battvt, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['Time(s)', 'Voltage(V)', 'Current(A)', 'TEMP (C)', 'Battery_%'])
        
        self.csv_afe1 = f"/home/vagrant/shared/fwxv/smoke/bms_carrier/csv_data/AFE_1_{timestamp}.csv"
        with open(self.csv_afe1, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([
                'Time(s)',
                'CELL VOLTAGE 1 (100 uV)',
                'CELL VOLTAGE 2 (100 uV)',
                'CELL VOLTAGE 3 (100 uV)',
                'CELL VOLTAGE 4 (100 uV)',
                'CELL VOLTAGE 5 (100 uV)',
                'CELL VOLTAGE 6 (100 uV)',
                'CELL VOLTAGE 7 (100 uV)',
                'CELL VOLTAGE 8 (100 uV)',
                'CELL VOLTAGE 9 (100 uV)',
                'CELL VOLTAGE 10 (100 uV)',
                'CELL VOLTAGE 11 (100 uV)',
                'CELL VOLTAGE 12 (100 uV)',
                'Temperature 1(C)',
                'Temperature 2(C)',
                'Temperature 3(C)'])
        
        self.csv_afe2 = f"/home/vagrant/shared/fwxv/smoke/bms_carrier/csv_data/AFE_2_{timestamp}.csv"
        with open(self.csv_afe2, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([
                'Time(s)',
                'CELL VOLTAGE 1 (100 uV)',
                'CELL VOLTAGE 2 (100 uV)',
                'CELL VOLTAGE 3 (100 uV)',
                'CELL VOLTAGE 4 (100 uV)',
                'CELL VOLTAGE 5 (100 uV)',
                'CELL VOLTAGE 6 (100 uV)',
                'CELL VOLTAGE 7 (100 uV)',
                'CELL VOLTAGE 8 (100 uV)',
                'CELL VOLTAGE 9 (100 uV)',
                'CELL VOLTAGE 10 (100 uV)',
                'CELL VOLTAGE 11 (100 uV)',
                'CELL VOLTAGE 12 (100 uV)',
                'Temperature 1(C)',
                'Temperature 2(C)',
                'Temperature 3(C)'])
        
        self.csv_afe3 = f"/home/vagrant/shared/fwxv/smoke/bms_carrier/csv_data/AFE_3_{timestamp}.csv"
        with open(self.csv_afe3, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([
                'Time(s)',
                'CELL VOLTAGE 1 (100 uV)',
                'CELL VOLTAGE 2 (100 uV)',
                'CELL VOLTAGE 3 (100 uV)',
                'CELL VOLTAGE 4 (100 uV)',
                'CELL VOLTAGE 5 (100 uV)',
                'CELL VOLTAGE 6 (100 uV)',
                'CELL VOLTAGE 7 (100 uV)',
                'CELL VOLTAGE 8 (100 uV)',
                'CELL VOLTAGE 9 (100 uV)',
                'CELL VOLTAGE 10 (100 uV)',
                'CELL VOLTAGE 11 (100 uV)',
                'CELL VOLTAGE 12 (100 uV)',
                'Temperature 1(C)',
                'Temperature 2(C)',
                'Temperature 3(C)'])

        self.notifier = can.Notifier(self.can_bus, [self.handle_bms_message])
    
    def __del__(self):
        print("Cleaning up...")

    def handle_bms_message(self, msg):
        """Handle incoming CAN messages"""
        try:
            print(msg)
            decoded_msg = self.db.decode_message(msg.arbitration_id, msg.data)
            if msg.arbitration_id == 1925 or msg.arbitration_id == 1957 or msg.arbitration_id == 1989:
                id = decoded_msg.get('id', 0)
                if id != self.last_id:
                    self.last_id = id
                    self.afe1_sampled = False
                    self.afe2_sampled = False
                    self.afe3_sampled = False
                
            current_time = time.time() - self.start_time
            print(decoded_msg)
            if msg.arbitration_id == 15:
                voltage = decoded_msg.get('voltage', 0)
                current = decoded_msg.get('current', 0)
                if current > 32767:
                    current = current - 65536
                temperature = decoded_msg.get('temperature', 0)
                batt_perc = decoded_msg.get('batt_perc', 0)
                self.write_batt_vt_csv(self.csv_battvt, current_time, voltage, current, temperature, batt_perc)

            # AFE 1   
            if msg.arbitration_id == 1925 and self.afe1_sampled == False:
                self.afe1_sampled = True
                v1 = decoded_msg.get('v1', 0)
                v2 = decoded_msg.get('v2', 0)
                v3 = decoded_msg.get('v3', 0)
                temp = decoded_msg.get('temp', 0)

                self.cell_voltages_1.append(v1)
                self.cell_voltages_1.append(v2)
                self.cell_voltages_1.append(v3)
                self.temperatures_1.append(temp)

                id = decoded_msg.get('id', 0)
                if id == 3:
                    self.write_afe_csv(self.csv_afe1, current_time, self.cell_voltages_1, self.temperatures_1)
                    self.cell_voltages_1 = []
                    self.temperatures_1 = []   

            # AFE 2
            if msg.arbitration_id == 1957 and self.afe2_sampled == False:
                self.afe2_sampled = True
                v1 = decoded_msg.get('v1', 0)
                v2 = decoded_msg.get('v2', 0)
                v3 = decoded_msg.get('v3', 0)
                temp = decoded_msg.get('temp', 0)

                self.cell_voltages_2.append(v1)
                self.cell_voltages_2.append(v2)
                self.cell_voltages_2.append(v3)
                self.temperatures_2.append(temp)

                id = decoded_msg.get('id', 0)
                if id == 3:
                    self.write_afe_csv(self.csv_afe2, current_time, self.cell_voltages_2, self.temperatures_2)
                    self.cell_voltages_2 = []
                    self.temperatures_2 = []

            # AFE 3
            if msg.arbitration_id == 1989 and self.afe3_sampled == False:
                self.afe3_sampled = True
                v1 = decoded_msg.get('v1', 0)
                v2 = decoded_msg.get('v2', 0)
                v3 = decoded_msg.get('v3', 0)
                temp = decoded_msg.get('temp', 0)

                self.cell_voltages_3.append(v1)
                self.cell_voltages_3.append(v2)
                self.cell_voltages_3.append(v3)
                self.temperatures_3.append(temp)

                id = decoded_msg.get('id', 0)
                if id == 3:
                    self.write_afe_csv(self.csv_afe3, current_time, self.cell_voltages_3, self.temperatures_3)
                    self.cell_voltages_3 = []
                    self.temperatures_3 = []
            
        except KeyError as e:
            print(f"Error decoding message: {e}")
            
        except Exception as e:
            print(f"Unexpected error: {e}")

    def write_batt_vt_csv(self, csv_file, time, voltage, current, temperature, batt_perc):
        with open(csv_file, 'a', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([f"{time:.3f}", voltage, current, temperature, batt_perc])

    
    def write_afe_csv(self, csv_file, time, cell_voltages, cell_temperatures):
        with open(csv_file, 'a', newline='') as csvfile:
            writer = csv.writer(csvfile)
            row = [f"{time:.3f}"]
            row.extend(cell_voltages)
            row.extend(cell_temperatures)
            writer.writerow(row)


if __name__ == "__main__":
    bms = BMSListener()