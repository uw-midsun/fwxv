'''
python3 /home/vagrant/shared/fwxv/libraries/codegen/generator.py -y /home/vagrant/shared/fwxv/libraries/codegen/boards/boards.yaml -f /home/vagrant/shared/fwxv/gen -t /home/vagrant/shared/fwxv/libraries/codegen/templates/boards.py.jinja

'''



import subprocess
from time import sleep
import struct

DRIVE_OUTPUT = (9 << 5) + 2
PEDAL_OUTPUT = (18 << 5) + 5

def msg(id, data):
    print(f"cansend can0 {id:x}#{data:x}")
    subprocess.run(f"cansend can0 {id:x}#{data}", shell=True)

def pack_drive(drive_state=0, target_vel=0, regen=0, cruise=0):
    return f"{struct.pack('<f', target_vel).hex()}{drive_state:02x}{cruise:02x}{regen:02x}"

def pack_pedal(throttle=0, brake=0):
    return struct.pack('<f', throttle).hex() + struct.pack('<f', brake).hex()

msg(DRIVE_OUTPUT, pack_drive(0, cruise=1, regen=1, target_vel=25.5))
msg(PEDAL_OUTPUT, pack_pedal(0.4, 0.1))
