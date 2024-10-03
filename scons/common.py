import json
import subprocess
import serial  # pip install pyserial
import glob
from sys import platform

def parse_config(entry):
    # Default config to empty for fields that don't exist
    ret = {
        'libs': [],
        'x86_libs': [],
        'arm_libs': [],
        'cflags': [],
        'mocks': {},
        'no_lint': False,
        'can': False,
        'arm_only': False
    }
    config_file = entry.File('config.json')
    if not config_file.exists():
        return ret
    with open(config_file.abspath, 'r') as f:
        config = json.load(f)
        for key, value in config.items():
            ret[key] = value
    return ret


def flash_run(entry, flash_type):
    '''flash and run file, return a pyserial object which monitors the device serial output'''
    try:
        output = subprocess.check_output(["ls", "/dev/serial/by-id/"])
        device_path = f"/dev/serial/by-id/{str(output, 'ASCII').strip()}"
        serialData = serial.Serial(device_path, 115200)
    except:
        print()
        print("Flashing requires a controller board to be connected, use --platform=x86 for x86 targets")

    OPENOCD = 'openocd'
    OPENOCD_SCRIPT_DIR = '/usr/share/openocd/scripts/'
    PROBE = 'cmsis-dap'
    PLATFORM_DIR = 'platform'
    OPENOCD_CFG = [
        OPENOCD,
        '-s {}'.format(OPENOCD_SCRIPT_DIR),
        '-f interface/{}.cfg'.format(PROBE),
        '-f target/stm32f1x.cfg',
        '-f {}/stm32f1-openocd.cfg'.format(PLATFORM_DIR),
        '-c "stm32f1x.cpu configure -rtos FreeRTOS"',
    ]
    if flash_type == 'default':
        OPENOCD_CFG += [
            '-c "stm_flash {}"'.format(entry),
        ]
    elif flash_type == 'application':
        OPENOCD_CFG+= [
            '-c "stm_flash_bootloader_application {}"'.format(entry),
        ]
    elif flash_type == 'bootloader':
        OPENOCD_CFG += [
            '-c "stm_flash_bootloader {}"'.format(entry),
        ]
    elif flash_type == 'erase_flash':
        OPENOCD_CFG += [
            '-c "stm_erase"',
        ]
    else:
        raise ValueError('Invalid flash_type')
    
    OPENOCD_CFG.append('-c shutdown')

    cmd = 'sudo {}'.format(' '.join(OPENOCD_CFG))

    subprocess.run(cmd, shell=True).check_returncode()

    print("Flash complete")
