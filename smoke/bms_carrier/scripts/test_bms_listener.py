import can
import cantools
import numpy as np
import time
from bms_listener import BMSListener


class TestBMSListener:
    def __init__(self):
        self.dbc_file = '/home/firmware/dev/Akashem06/fwxv/smoke/bms_carrier/scripts/system_can.dbc'
        self.db = cantools.database.load_file(self.dbc_file)
        self.can_bus = can.interface.Bus(channel='vcan0', bustype='virtual')

        self.listener = BMSListener()
        self.listener.db = self.db
        self.listener.can_bus = self.can_bus

        # Test parameters
        self.test_voltage = 350
        self.test_current = 150
        self.test_temperature = 25
        self.test_batt_perc = 80

        self.READINGS_PER_MSG = 3
        self.NUM_MSG = 4
        self.test_afe_message_index = 0

        # Generate mock data
        self.mock_cell_voltages = []
        self.mock_temp = []

        # Generate mock cell voltages (33.3V ± 0.05V)
        for _ in range(12 * 3):  # 3 AFEs, 12 cells each
            voltage = 33300 + np.random.randint(-500, 500)  # 100 uV
            self.mock_cell_voltages.append(voltage)

        # Generate mock temperatures (25°C ± 5°C)
        for _ in range(6 * 3):  # 6 thermistors per AFE, 3 AFEs
            temp = 25 + np.random.randint(-5, 5)
            self.mock_temp.append(temp)

    def mock_afe_messages(self):
        """Mock AFE messages with voltage and temperature data"""
        try:
            # Calculate base indices for each AFE's data
            read_index = self.test_afe_message_index * self.READINGS_PER_MSG
            read_index2 = 12 + self.test_afe_message_index * self.READINGS_PER_MSG
            read_index3 = 24 + self.test_afe_message_index * self.READINGS_PER_MSG

            # Define message configurations
            afe_configs = [
                {
                    'base_id': 1925,
                    'name': 'AFE1_status',
                    'voltages': self.mock_cell_voltages[read_index:read_index + 3],
                    'temp_index': self.test_afe_message_index
                },
                {
                    'base_id': 1957,
                    'name': 'AFE2_status',
                    'voltages': self.mock_cell_voltages[read_index2:read_index2 + 3],
                    'temp_index': 6 + self.test_afe_message_index
                },
                {
                    'base_id': 1989,
                    'name': 'AFE3_status',
                    'voltages': self.mock_cell_voltages[read_index3:read_index3 + 3],
                    'temp_index': 12 + self.test_afe_message_index
                }
            ]

            # Send messages for each AFE
            for config in afe_configs:
                msg_id = config['base_id']

                # Prepare data dictionary
                data = {
                    'id': self.test_afe_message_index,
                    'v1': config['voltages'][0],
                    'v2': config['voltages'][1],
                    'v3': config['voltages'][2]
                }

                # Only include temperature in first three messages of each group
                if self.test_afe_message_index < 4:
                    data['temp'] = self.mock_temp[config['temp_index']]
                # Create and send CAN message
                msg = can.Message(
                    arbitration_id=msg_id,
                    data=self.db.encode_message(msg_id, data),
                    is_extended_id=False
                )
                self.can_bus.send(msg)

            self.test_afe_message_index = (self.test_afe_message_index + 1) % self.NUM_MSG

        except Exception as e:
            print(f"Error sending AFE messages: {e}")
            raise

    def mock_battery_message(self):
        """Mock battery voltage/current/temperature message"""
        try:
            data = {
                'voltage': self.test_voltage,
                'current': self.test_current,
                'temperature': self.test_temperature,
                'batt_perc': self.test_batt_perc
            }

            msg = can.Message(
                arbitration_id=15,  # battery_vt message ID
                data=self.db.encode_message(15, data),
                is_extended_id=False
            )
            self.can_bus.send(msg)

        except Exception as e:
            print(f"Error sending battery message: {e}")
            raise

    def run_test(self):
        """Run the test loop"""
        print("Starting BMS test...")
        try:
            while True:
                self.mock_battery_message()
                self.mock_afe_messages()
                time.sleep(1)  # Wait 1 second between messages

        except KeyboardInterrupt:
            print("\nTest stopped by user")
        except Exception as e:
            print(f"Test error: {e}")
        finally:
            print("Shutting down...")
            self.listener.__del__()  # Clean up the listener
            self.can_bus.shutdown()


if __name__ == "__main__":
    test = TestBMSListener()
    test.run_test()
