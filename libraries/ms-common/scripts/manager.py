"""
This module contains the x86 car simulations manager.
It allows you to interact with the vehicles features.
"""

import os
import socket
import time
import subprocess
import can

################################################################################
# INITIALIZATION

PROJECTS = [
    "bms_carrier", "centre_console",
    "motor_controller", "pedal",
    "power_distribution", "steering"
]
# PROJECTS = ["power_distribution", "leds"]
PROJECT_DIR = os.path.join(os.getcwd(), 'projects')
BUILD_DIR = os.path.join(os.getcwd(), 'build/x86/bin/projects')
BUS = can.interface.Bus('vcan0', bustype='virtual')

try:
    subprocess.run(['sudo', 'ip', 'link', 'add', 'dev', 'vcan0', 'type', 'vcan'], check=True)
    subprocess.run(['sudo', 'ip', 'link', 'set', 'up', 'vcan0'], check=True)
except Exception as error:
    print(f"Error: {error}")

"""
Project manager class. Each project/binary has its own manager with a unique socket number.
"""


class ProjectManager:
    def __init__(self, project, socket_num):
        self.project = project
        self.socket_num = socket_num

    def start_project(self):
        if os.path.exists(os.path.join(PROJECT_DIR, self.project)):
            program = os.path.join(BUILD_DIR, self.project)
        else:
            print(f"ERROR: Project file not found for {self.project}. Doesn't exist")
            return
        try:
            subprocess.run(["scons", "--platform=x86", "--project=" + self.project], check=True)
            subprocess.Popen(
                [program, str(self.socket_num)],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE
            )
            time.sleep(2)
            self.manager_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.manager_socket.connect(('127.0.0.1', self.socket_num))
            print("ACTIVE ON PORT: ", self.socket_num)
        except Exception as error:
            print(f"ERROR: Socket failed. {error}")
            return
        print("PROJECT STARTED")

        return self.socket_num

    def call_operation(self, operations):
        try:
            for op in operations:
                self.manager_socket.send(op.encode())
        except Exception as error:
            print(f"ERROR: {error}")

    def read_operation(self):
        try:
            msg = (self.manager_socket.recv(2048)).decode()
            print(msg)
        except Exception as error:
            print(f"ERROR: {error}")

    def __del__(self):
        if self.manager_socket:
            self.manager_socket.close()

################################################################################
# CAR CLASS


class Car:
    def __init__(self, projects):
        self.managers = {}
        socket_determine = 1024
        for project in projects:
            socket_determine += 1
            self.managers[project] = ProjectManager(project, socket_determine)

    def start_projects(self):
        socket_nums = {}
        for project, manager in self.managers.items():
            socket_nums[project] = manager.start_project()
        return socket_nums

    def operate(self, project, operations):  # operation
        self.managers[project].call_operation(operations)

    def read(self, project):
        self.managers[project].read_operation()

    def __del__(self):
        for manager in self.managers.values():
            del manager


################################################################################
# MAKE CAR
CAR = Car(projects)
# Wrappers

"""
Run this command at the very beginning.
Starts each projects binaries and establishes socket communication.
"""


def start_car():
    socket_nums = CAR.start_projects()


"""
Allows you to set a GPIO pins state.
"""


def gpio_set_state(project):
    pin_letter = input("Select your GPIO pin letter (0-2): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    state = input("Select if it is high or low (0/1): ")
    CAR.operate(str(project), f"0: 3, {pin_letter}, {pin_num}, {state}\n")


"""
Toggles a GPIO pin
"""


def gpio_toggle(project):
    pin_letter = input("Select your GPIO pin letter (0-2): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    CAR.operate(str(project), f"1: 2, {pin_letter}, {pin_num}\n")


"""
Toggles an interrupt on a GPIO pin
"""


def gpio_toggle_it(project):
    pin_letter = input("Select your GPIO pin letter (0-3): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    CAR.operate(str(project), f"2: 2, {pin_letter}, {pin_num}\n")


"""
Sets the ADC reading value
"""


def adc_set_reading(project):
    pin_letter = input("Select your GPIO pin letter (0-3): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    adc_reading = input("Enter your ADC reading (x-y): ")
    CAR.operate(str(project), f"3: 3, {pin_letter}, {pin_num}, {adc_reading}\n")


"""
Sets the i2c reading value
"""


def i2c_set_reading(project):
    i2c_port = input("Select your I2C port (0/1): ")
    i2c_data_size = input("Enter the size of your message: ")
    i2c_data = input("Enter the data to write: ")
    CAR.operate(str(project), f"4: 3, {i2c_port}, {i2c_data_size}, {i2c_data}\n")


"""
Sets the spi rx value
"""


def spi_set_rx(project):
    spi_port = input("Select your SPI port (0/1): ")
    spi_data_size = input("Enter the size of your message")
    spi_data = input("Enter the data to write: ")
    CAR.operate(str(project), f"5: 3, {spi_port}, {spi_data_size}, {spi_data}\n")

##########################################################
# Incomplete library
##########################################################


def uart(project):
    print("x86 sim portion must be made")

##########################################################
# Reads out all pins state and mode.
##########################################################


def gpio_read(project):
    CAR.operate(str(project), f"7: 0\n")
    time.sleep(1)
    CAR.read(str(project))
