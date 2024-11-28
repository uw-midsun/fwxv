import matplotlib.pyplot as plt
import numpy as np
import time
import csv


class DischargeTestGraph:
    def __init__(self, discharge_current, save_interval=60):
        self.discharge_current = discharge_current

        self.voltage_readings = np.array([], dtype=float)
        self.time_intervals = np.array([], dtype=float)
        self.batt_ah = np.array([], dtype=float)

        plt.ion()
        self.fig, (self.ax1, self.ax2) = plt.subplots(1, 2, figsize=(17.5, 8))
        self.fig.subplots_adjust(left=0.05, right=0.95, wspace=0.3)

        self.start_time = time.time()
        self.last_save_time = self.start_time
        self.save_interval = save_interval
        self.data_file_counter = 0

        self.csv_filename = f'csv_data/discharge_data_{self.data_file_counter}.csv'

        plt.get_current_fig_manager().set_window_title("Discharge Curves")

    def save_to_csv(self):
        with open(self.csv_filename, mode='w', newline='') as csv_file:
            csv_writer = csv.writer(csv_file)

            csv_writer.writerow(['Time (s)', 'Voltage (V)', 'Amp-hours (Ah)'])
            for i in range(len(self.time_intervals)):
                csv_writer.writerow(
                    [self.time_intervals[i], self.voltage_readings[i], self.batt_ah[i]])

    def update_plots(self):
        self.ax1.clear()
        self.ax1.plot(
            self.time_intervals,
            self.voltage_readings,
            label='Voltage (V)',
            color='green')
        self.ax1.set_xlabel('Time (s)')
        self.ax1.set_ylabel('Voltage (V)')
        self.ax1.set_title('Discharge Curve of Pack against Time')
        self.ax1.grid(True)
        self.ax1.legend()

        self.ax2.clear()
        self.ax2.plot(self.batt_ah, self.voltage_readings, label='Voltage (V)', color='orange')
        self.ax2.set_xlabel('Amp-hours (Ah)')
        self.ax2.set_ylabel('Voltage (V)')
        self.ax2.set_title('Discharge Curve of Pack against Ah')
        self.ax2.grid(True)
        self.ax2.legend()

        plt.pause(0.1)

    def add_data(self, voltage, current_time):
        time_elapsed = current_time - self.start_time

        self.time_intervals = np.append(self.time_intervals, time_elapsed)
        self.voltage_readings = np.append(self.voltage_readings, voltage)

        if len(self.time_intervals) > 1:
            delta_time = (self.time_intervals[-1] - self.time_intervals[-2]) / 3600
            delta_ah = self.discharge_current * delta_time
            self.batt_ah = np.append(self.batt_ah,
                                     self.batt_ah[-1] + delta_ah if len(self.batt_ah) > 0 else delta_ah)
        else:
            self.batt_ah = np.append(self.batt_ah, 0)

        self.update_plots()

        if current_time - self.last_save_time >= self.save_interval:
            self.save_to_csv()
            self.last_save_time = current_time

            # Clear memory
            if (len(self.time_intervals) >= 200000):  # 55 Hours worth of data
                self.data_file_counter += 1
                self.voltage_readings = np.array([], dtype=float)
                self.time_intervals = np.array([], dtype=float)
                self.batt_ah = np.array([], dtype=float)

    def stop_plotting(self):
        self.save_to_csv()
        plt.ioff()
        plt.show()
