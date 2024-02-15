import sys
import signal
import os
import socket
import time
import subprocess
import can
from random import randint

################################################################################
# INITIALIZATION

projects = [ "centre_console" ]
BUILD_DIR = os.path.join(os.getcwd(), 'build/x86/bin/projects')
bus = can.interface.Bus('vcan0', bustype='virtual') 
subprocess.run(['sudo', 'ip', 'link', 'add', 'dev', 'vcan0', 'type', 'vcan'])
subprocess.run(['sudo', 'ip', 'link', 'set', 'up', 'vcan0'])

################################################################################
# PROJECT CLASS

class ProjectManager:
  def __init__(self, project, socket_num):
    self.project = project
    self.socket_num = socket_num
  
  def start_project(self):
    if os.path.exists(os.path.join(BUILD_DIR, self.project)):
      program = os.path.join(BUILD_DIR, self.project)
    else:
      print(f"ERROR: Project file not found for {self.project}. MUST BE x86 BUILD")
      return
    subprocess.run(["scons", "--platform=x86", "--project="+self.project])
    subprocess.Popen([program, str(self.socket_num)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(1)
    try:
      self.manager_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.manager_socket.connect(('127.0.0.1', self.socket_num))
    except Exception as e:
      print(f"ERROR: Socket failed. {e}")
      return
    print("PROJECT STARTED")
    
    return self.socket_num

  def call_operation(self, operations):
    try:
      for op in operations:
          self.manager_socket.send(op.encode())
    except Exception as e:
      print(f"ERROR: {e}")
    
  def read_operation(self):
    try:
      msg = (self.manager_socket.recv(2048)).decode()
      print(msg)
    except Exception as e:
      print(f"ERROR: {e}")
    
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
    
  def operate(self, project, operations): # operation
    self.managers[project].call_operation(operations)

  def read(self, project):
    self.managers[project].read_operation()
  
  def __del__(self):
    for manager in self.managers.values():
        del manager

################################################################################
# MAKE CAR
car = Car(projects)
### Wrappers
def start_car():
  socket_nums = car.start_projects()

def gpio_set_state(project):
  pinLetter = input("Select your GPIO pin letter (0-3): ")
  pinNum = input ("Select your GPIO pin number (0-15): ")
  state = input("Select if it is high or low (0/1): ")
  car.operate(str(project), f"0: 3, {pinLetter}, {pinNum}, {state}\n")

def gpio_toggle(project):
  pinLetter = input("Select your GPIO pin letter (0-3): ")
  pinNum = input ("Select your GPIO pin number (0-15): ")
  car.operate(str(project), f"1: 2, {pinLetter}, {pinNum}\n")

def gpio_toggle_it(project):
  pinLetter = input("Select your GPIO pin letter (0-3): ")
  pinNum = input ("Select your GPIO pin number (0-15): ")
  car.operate(str(project), f"2: 2, {pinLetter}, {pinNum}\n")

def gpio_read(project):
  car.operate(str(project), f"7: 0\n")
  time.sleep(1)
  car.read(str(project))

def test_message():
  car.read("centre_console")
### Tests
