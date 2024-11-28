import time
import numpy as np
import random
from discharge_graph import DischargeTestGraph


def mock_voltage_discharge_curve(initial_voltage, final_voltage, discharge_duration,
                                 num_points, voltage_variation=0.05, time_variation=0.05):
    time_intervals = np.linspace(0, discharge_duration, num_points)
    voltage_values = np.linspace(initial_voltage, final_voltage, num_points)

    # Add random variations to voltage and time
    voltage_values = np.array(
        [v + random.uniform(-voltage_variation, voltage_variation) for v in voltage_values])
    time_intervals = np.array([t + random.uniform(-time_variation, time_variation)
                               for t in time_intervals])

    time_intervals = np.maximum.accumulate(time_intervals)

    return time_intervals, voltage_values


def test_discharge_test_graph():
    discharge_current = 2.5
    discharge_duration = 60 * 60 * 2
    initial_voltage = 4.2
    final_voltage = 2.5
    num_points = 200
    save_interval = 60
    voltage_variation = 0.015
    time_variation = 1

    discharge_test_graph = DischargeTestGraph(discharge_current, save_interval)

    time_intervals, voltage_values = mock_voltage_discharge_curve(
        initial_voltage, final_voltage, discharge_duration, num_points, voltage_variation, time_variation)

    start_time = time.time()

    for i in range(len(time_intervals)):
        current_time = start_time + time_intervals[i]
        voltage = voltage_values[i]

        discharge_test_graph.add_data(voltage, current_time)

        time.sleep(0.05)

    # Stop the plotting
    discharge_test_graph.stop_plotting()


if __name__ == "__main__":
    test_discharge_test_graph()
