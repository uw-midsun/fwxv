'''
This module contains the x86 car simulations manager.
It allows you to interact with the vehicles features.
'''

import os
import socket
import time
import subprocess

################################################################################
# INITIALIZATION

PROJECTS = [
    "bms_carrier", "centre_console",
    # "motor_controller", "pedal", KNOWN ISSUES WITH THIS? SEGMENTATION FAULT
    "power_distribution", "steering"
]
# PROJECTS = ["pedal"]
PROJECT_DIR = os.path.join(os.getcwd(), 'projects')
BUILD_DIR = os.path.join(os.getcwd(), 'build/x86/bin/projects')

try:
    subprocess.run(['sudo', 'ip', 'link', 'add', 'dev', 'vcan0', 'type', 'vcan'], check=True)
    subprocess.run(['sudo', 'ip', 'link', 'set', 'up', 'vcan0'], check=True)
except Exception as error:
    print(f"Error: {error}")

class ProjectManager:
    '''
    Docfile
    '''
    def __init__(self, project, socket_num):
        self.project = project
        self.socket_num = socket_num
        self.manager_socket = 0

    def start_project(self):
        '''
        Docfile
        '''
        if os.path.exists(os.path.join(PROJECT_DIR, self.project)):
            program = os.path.join(BUILD_DIR, self.project)
        else:
            print(f"ERROR: Project file not found for {self.project}. Doesn't exist")
            return 0
        try:
            self.socket_num = self.find_available_port()
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
            return 0
        print("PROJECT STARTED")

        return self.socket_num

    def find_available_port(self):
        '''
        Find an available port starting from the given port number.
        '''
        while self.socket_num < 65535:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            result = sock.connect_ex(('127.0.0.1', self.socket_num))
            sock.close()
            if result != 0:
                return self.socket_num
            self.socket_num += 1
        return None

    def call_operation(self, operations):
        '''
        Docfile
        '''
        try:
            for char in operations:
                self.manager_socket.send(char.encode())
        except Exception as error:
            print(f"ERROR: {error}")

    def read_operation(self):
        '''
        Docfile
        '''
        try:
            msg = (self.manager_socket.recv(2048)).decode()
            print(msg)
        except Exception as error:
            print(f"ERROR: {error}")

    def __del__(self):
        '''
        Docfile
        '''
        if self.manager_socket:
            self.manager_socket.close()

################################################################################
# CAR CLASS


class Car:
    '''
    Docfile
    '''
    def __init__(self, projects):
        self.managers = {}
        minimum_port = 1024
        for project in projects:
            self.managers[project] = ProjectManager(project, minimum_port)

    def start_projects(self):
        '''
        Docfile
        '''
        socket_nums = {}
        for project, manager in self.managers.items():
            socket_nums[project] = manager.start_project()
        return socket_nums

    def operate(self, project, operations):
        '''
        Docfile
        '''
        self.managers[project].call_operation(operations)

    def read(self, project):
        '''
        Docfile
        '''
        self.managers[project].read_operation()

    def __del__(self):
        for manager in self.managers.values():
            del manager


################################################################################
# MAKE CAR
CAR = Car(PROJECTS)
# Wrappers

def start_car():
    '''
    Docfile
    '''
    CAR.start_projects()

def gpio_set_state(project):
    '''
    Docfile
    '''
    pin_letter = input("Select your GPIO pin letter (0-2): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    state = input("Select if it is high or low (0/1): ")
    CAR.operate(str(project), f"0: 3, {pin_letter}, {pin_num}, {state}\n")

def gpio_toggle(project):
    '''
    Docfile
    '''
    pin_letter = input("Select your GPIO pin letter (0-2): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    CAR.operate(str(project), f"1: 2, {pin_letter}, {pin_num}\n")

def gpio_toggle_it(project):
    '''
    Docfile
    '''
    pin_letter = input("Select your GPIO pin letter (0-3): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    CAR.operate(str(project), f"2: 2, {pin_letter}, {pin_num}\n")

def adc_set_reading(project):
    '''
    Docfile
    '''
    pin_letter = input("Select your GPIO pin letter (0-3): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    adc_reading = input("Enter your ADC reading (x-y): ")
    CAR.operate(str(project), f"3: 3, {pin_letter}, {pin_num}, {adc_reading}\n")

def i2c_set_reading(project):
    '''
    Docfile
    '''
    i2c_port = input("Select your I2C port (0/1): ")
    i2c_data_size = input("Enter the size of your message: ")
    i2c_data = input("Enter the data to write: ")
    CAR.operate(str(project), f"4: 3, {i2c_port}, {i2c_data_size}, {i2c_data}\n")

def spi_set_rx(project):
    '''
    Docfile
    '''
    spi_port = input("Select your SPI port (0/1): ")
    spi_data_size = input("Enter the size of your message")
    spi_data = input("Enter the data to write: ")
    CAR.operate(str(project), f"5: 3, {spi_port}, {spi_data_size}, {spi_data}\n")

def uart(project):
    '''
    Docfile
    '''
    print("x86 sim portion must be made")

def gpio_read(project):
    '''
    Docfile
    '''
    CAR.operate(str(project), f"7: 0\n")
    time.sleep(1)
    CAR.read(str(project))
