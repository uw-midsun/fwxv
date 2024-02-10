import sys
import signal
import os
import socket
import subprocess
import can
from random import randint

projects = [ "centre_console" ]
BUILD_DIR = os.path.join(os.getcwd(), 'build/x86/bin/projects')
bus = can.interface.Bus('vcan0', bustype='virtual') 
subprocess.run(['sudo', 'ip', 'link', 'add', 'dev', 'vcan0', 'type', 'vcan'])
subprocess.run(['sudo', 'ip', 'link', 'set', 'up', 'vcan0'])

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
    # subprocess.run([program, str(self.socket_num)], shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    try:
      self.manager_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.manager_socket.connect(('127.0.0.1', self.socket_num))
    except Exception as e:
      print(f"ERROR: {e}")
    
    return self.socket_num

  def call_operation(self, operations):
    try:
      for op in operations:
          self.manager_socket.send(op.encode())
          print(op.encode())
    except Exception as e:
      print(f"ERROR: {e}")
      

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
    
  # def cleanup(self):
  #     for project in self.managers:
  #         subprocess.run(["sudo", "killall", "-9", project])

  # def __del__(self):
  #     self.cleanup()
      
### Tests
car = Car(projects)
socket_nums = car.start_projects()
car.operate("centre_console", "GPIO_SET")