'''
This module contains the x86 car simulations manager.
It allows you to interact with the vehicles features.
'''

import os
import socket
import time
import subprocess

###########################################################
# INITIALIZATION
###########################################################

PROJECTS = [
    "bms_carrier", "centre_console",
    # "motor_controller", "pedal", KNOWN ISSUES WITH THIS? SEGMENTATION FAULT
    "power_distribution", "steering"
]
PROJECT_DIR = os.path.join(os.getcwd(), 'projects')
BUILD_DIR = os.path.join(os.getcwd(), 'build/x86/bin/projects')

try:
    subprocess.run(['sudo', 'ip', 'link', 'add', 'dev', 'vcan0', 'type', 'vcan'], check=True)
    subprocess.run(['sudo', 'ip', 'link', 'set', 'up', 'vcan0'], check=True)
except subprocess.CalledProcessError as error:
    print(f"Error: {error}")

###########################################################
# MANAGER CLASS FOR EACH PROJECT
###########################################################


class ProjectManager:
    '''
    This class is a wrapper for each project. It has has a unique
    port number used to communicate to the project
    '''

    def __init__(self, project, socket_num):
        self.project = project
        self.socket_num = socket_num
        self.manager_socket = 0

    def start_project(self):
        '''
        This function builds the x86 binary and starts the project.
        It also passes a port number into the binary to initialize
        the socket on both sides to start communication
        '''
        if os.path.exists(os.path.join(PROJECT_DIR, self.project)):
            program = os.path.join(BUILD_DIR, self.project)
        else:
            print(f"ERROR: Project file not found for {self.project}. Doesn't exist")
            return 0
        try:
            self.socket_num = self.find_available_port()
            subprocess.run(["scons", "--platform=x86", "--project=" + self.project], check=True)

            # CHANGE THIS TO THREADING INSTEAD... I hate scons lint.
            subprocess.Popen([program, str(self.socket_num)],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            time.sleep(2)
            self.manager_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.manager_socket.connect(('127.0.0.1', self.socket_num))
            print("ACTIVE ON PORT: ", self.socket_num)
        except subprocess.CalledProcessError as error:
            print(f"ERROR: Subprocess failed. {error}")
            return 0
        except socket.error as error:
            print(f"ERROR: Socket failed. {error}")
            return 0
        print("PROJECT STARTED")

        return self.socket_num

    def find_available_port(self):
        '''
        Find an available port starting from the given port number
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
        Handles socket TX. Typically sends operation requests by
        sending each character to the binary, through the socket
        '''
        try:
            for char in operations:
                self.manager_socket.send(char.encode())
        except socket.error as error:
            print(f"ERROR: {error}")

    def read_operation(self):
        '''
        Handles socket RX. Receives a message in a 2048 buffer
        '''
        try:
            msg = (self.manager_socket.recv(2048)).decode()
            print(msg)
        except socket.error as error:
            print(f"ERROR: {error}")

    def __del__(self):
        '''
        Handles project being shutdown by freeing ports
        '''
        if self.manager_socket:
            self.manager_socket.close()

###########################################################
# CAR CLASS
###########################################################


class Car:
    '''
    This class acts as a wrapper for the entire vehicle's system.
    It holds each project and their respective port number. Interacts
    through the ProjectManager class
    '''

    def __init__(self, projects):
        self.managers = {}
        minimum_port = 1024
        for project in projects:
            self.managers[project] = ProjectManager(project, minimum_port)

    def start_projects(self):
        '''
        This establishes the socket communication between each project
        in the projects list
        '''
        socket_nums = {}
        for project, manager in self.managers.items():
            socket_nums[project] = manager.start_project()
        return socket_nums

    def operate(self, project, operations):
        '''
        Handles socket TX for respective project
        '''
        self.managers[project].call_operation(operations)

    def read(self, project):
        '''
        Handles socket RX for respective project
        '''
        self.managers[project].read_operation()

    def __del__(self):
        for manager in self.managers.values():
            del manager

###########################################################
# MAKE CAR
###########################################################


CAR = Car(PROJECTS)

###########################################################
# Wrapper Functions (User Interface)
###########################################################


def start_car():
    '''
    Establishes all sockets between client and x86 binaries
    '''
    CAR.start_projects()


def gpio_set_state(project):
    '''
    Allows user to modify a pins state (Only if it's push/pull)
    '''
    pin_letter = input("Select your GPIO pin letter (0-2): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    state = input("Select if it is high or low (0/1): ")
    CAR.operate(str(project), f"0: 3, {pin_letter}, {pin_num}, {state}\n")


def gpio_toggle(project):
    '''
    Allows user to toggle a pins state (Only if it's push/pull)
    '''
    pin_letter = input("Select your GPIO pin letter (0-2): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    CAR.operate(str(project), f"1: 2, {pin_letter}, {pin_num}\n")


def gpio_toggle_it(project):
    '''
    Allows user to trigger a gpio interrupt (Not tested yet)
    '''
    pin_letter = input("Select your GPIO pin letter (0-3): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    CAR.operate(str(project), f"2: 2, {pin_letter}, {pin_num}\n")


def adc_set_reading(project):
    '''
    Allows user to set an ADC reading (Not tested yet)
    '''
    pin_letter = input("Select your GPIO pin letter (0-3): ")
    pin_num = input("Select your GPIO pin number (0-15): ")
    adc_reading = input("Enter your ADC reading (x-y): ")
    CAR.operate(str(project), f"3: 3, {pin_letter}, {pin_num}, {adc_reading}\n")


def i2c_set_reading(project):
    '''
    Allows user to set an I2C read (Not tested yet)
    '''
    i2c_port = input("Select your I2C port (0/1): ")
    i2c_data_size = input("Enter the size of your message: ")
    i2c_data = input("Enter the data to write: ")
    CAR.operate(str(project), f"4: 3, {i2c_port}, {i2c_data_size}, {i2c_data}\n")


def spi_set_rx(project):
    '''
    Allows user to set a SPI read (Not tested yet)
    '''
    spi_port = input("Select your SPI port (0/1): ")
    spi_data_size = input("Enter the size of your message")
    spi_data = input("Enter the data to write: ")
    CAR.operate(str(project), f"5: 3, {spi_port}, {spi_data_size}, {spi_data}\n")


def uart(project):
    '''
    Allows user to do something with UART (No x86 Infrastructure yet)
    '''
    print("x86 sim portion must be made")


def gpio_read(project):
    '''
    Allows user to view all GPIO pin states/modes
    '''
    CAR.operate(str(project), "7: 0\n")
    time.sleep(1)
    CAR.read(str(project))
