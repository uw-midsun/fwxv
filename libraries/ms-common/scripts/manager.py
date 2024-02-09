import os
import socket
import subprocess
import time
from random import randint

projects = [ "centre_console", "bms_carrier", "power_distribution", "motor_controller", "pedal"]
BUILD_DIR = 'shared/fwxv/build/bin/projects/' 

class Car:
  def __init__(self, projects):
    self.managers = {}
    for project in projects:
      self.manager[project] = start_project(project)
      
  def start_project(self, project):
    if (os.path.isdir(BUILD_DIR + project)):
        program = os.path.join(BUILD_DIR, project)
    socket_num = randint() # generate random socket number
    while socket_num is not available:
      socket_num = randint() # keep generating until valid
    print(socket_num)

    s = socket.socket()
      
    subprocess.run([program, str(socket_num)]) # Start the project with the socket number
    return socket_num
    
  def call(self, project, operations): # operation
    socket = ....start socket connection with self.managers[project]
    ...or keep connection alive all the time??
    for op in operations:
      socket.send(op)
      
### Tests
def test_car_start():
  car = Car(projects)
  # car.call("centre_console", "connect battery.....")
  # Will need to look at logs to see if something went wrong
  
  