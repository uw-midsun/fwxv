import os
import socket
import time

project = [ "centre_console", "bms_carrier", "power_distribution", "motor_controller", "pedal"]
BUILD_DIR = 'shared/fwxv/build/bin/projects/' 

class Car:
  def __init__(self, projects):
    self.managers = {}
    for project in projects:
      self.manager[project] = start_project(project)
      
  def start_project(self, project):
    if (os.path.isdir(BUILD_DIR + project)):
        program = BUILD_DIR + project
    socket_num = rand() # generate random socket number
    while socket_num is not available:
      socket_num = rand() # keep generating until valid
      
    process.run(program, socket_num) # Start the project with the socket number
    # Maybe also redirect logs here?
    return socket_num
    
  def call(self, project, operations): # operation
    socket = ....start socket connection with self.managers[project]
    ...or keep connection alive all the time??
    for op in operations:
      socket.send(op)
      
### Tests
def test_car_start():
  car = Car(projects)
  car.call("centre_console", "connect battery.....")
  .....
  # Will need to look at logs to see if something went wrong
  
  