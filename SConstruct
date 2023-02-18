import os
import sys
import subprocess
from scons.common import parse_config
from scons.new_task import make_new_task

###########################################################
# Build arguments
###########################################################

AddOption(
    '--platform',
    dest='platform',
    type='string',
    action='store',
    default='arm'
)

AddOption(
    '--project',
    dest='project',
    type='string',
    action='store',
)

AddOption(
    '--library',
    dest='library',
    type='string',
    action='store',
)

AddOption(
    '--python',
    dest='python',
    type='string',
    action='store'
)
AddOption(
    '--py',
    dest='python',
    type='string',
    action='store'
)

AddOption(
    '--test',
    dest='testfile',
    type='string',
    action='store'
)

AddOption(
    '--define',
    dest='define',
    type='string',
    action='store'
)

AddOption(
    '--name',
    dest='name',
    type='string',
    action='store'
)

# Adding Memory Report Argument to Environment Flags
# Note platform needs to be explicitly set to arm

AddOption(
    '--mem-report',
    dest='mem-report',
    type='string',
    action='store',
)

PLATFORM = GetOption('platform')
PROJECT = GetOption('project')
LIBRARY = GetOption('library')
PYTHON = GetOption('python')
MEM_REPORT = GetOption('mem-report')
NAME = GetOption('name')

###########################################################
# Environment setup
###########################################################

# Retrieve the construction environment from the appropriate platform script
if PLATFORM == 'x86':
    env = SConscript('platform/x86.py')
elif PLATFORM == 'arm':
    env = SConscript('platform/arm.py')


# env.VariantDir('build', '.', duplicate=0)

TYPE = None
if PROJECT:
    TYPE = 'project'
elif LIBRARY:
    TYPE = 'library'
elif PYTHON:
    TYPE = 'python'
TARGET = PROJECT or LIBRARY or PYTHON

VARS = {
    "PLATFORM": PLATFORM,
    "TYPE": TYPE,
    "TARGET": TARGET,
    "NAME": NAME,
    "env": env,
}

env["VARS"] = VARS

# Add flags when compiling a test
TEST_CFLAGS = ['-DMS_TEST']
if 'test' in COMMAND_LINE_TARGETS: # are we running "scons test"?
    env['CCFLAGS'] += TEST_CFLAGS

# Parse asan / tsan and Adding Sanitizer Argument to Environment Flags
# Note platform needs to be explicitly set to x86

AddOption(
    '--sanitizer',
    dest='sanitizer',
    type='string',
    action='store',
    default="none"
)
SANITIZER = GetOption('sanitizer')

if SANITIZER == 'asan':
    env['CCFLAGS']   += ["-fsanitize=address"]
    env['CXXFLAGS']  += ["-fsanitize=address"]
    env['LINKFLAGS'] += ["-fsanitize=address"]
elif SANITIZER == 'tsan':
    env['CCFLAGS']   += ["-fsanitize=thread"]
    env['CXXFLAGS']  += ["-fsanitize=thread"]
    env['LINKFLAGS'] += ["-fsanitize=thread"]

env['CCCOMSTR']     = "Compiling  $TARGET"
env['LINKCOMSTR']   = "Linking    $TARGET"
env['ARCOMSTR']     = "Archiving  $TARGET"
env['ASCOMSTR']     = "Assembling $TARGET"
env['RANLIBCOMSTR'] = "Indexing   $TARGET"

###########################################################
# Directory setup
###########################################################

BUILD_DIR = Dir('#/build').Dir(PLATFORM)
BIN_DIR = BUILD_DIR.Dir('bin')
OBJ_DIR = BUILD_DIR.Dir('obj')

PROJ_DIR = Dir('#/projects')
SMOKE_DIR = Dir('#/smoke')

PLATFORM_DIR = Dir('#/platform')

VariantDir(OBJ_DIR, '.', duplicate=0)

###########################################################
# Build
###########################################################
SConscript('scons/build.scons', exports='VARS')

###########################################################
# Testing
###########################################################
SConscript('scons/test.scons', exports='VARS')

###########################################################
# Helper targets
###########################################################
SConscript('scons/new_target.scons', exports='VARS')
Alias('new_task', env.Command('new_task.txt', [], make_new_task))

###########################################################
# Clean
###########################################################
# 'clean.txt' is a dummy file that doesn't get created
# This is required for phony targets for scons to be happy
clean = Command('clean.txt', [], 'rm -rf build/*')
Alias('clean', clean)

###########################################################
# Linting and Formatting
###########################################################
SConscript('scons/lint_format.scons', exports='VARS')



# ELFs are used for gdb and x86
def proj_elf(proj_name, is_smoke=False):
    return BIN_DIR.Dir(SMOKE_DIR.name if is_smoke else PROJ_DIR.name).File(proj_name)

# .bin is used for flashing to MCU
def proj_bin(proj_name, is_smoke=False):
    return proj_elf(proj_name, is_smoke).File(proj_name + '.bin')

###########################################################
# Helper targets for x86
###########################################################
if PLATFORM == 'x86' and TYPE == 'project':
    # os.exec the x86 project ELF file to simulate it
    def sim_run(target, source, env):
        path = proj_elf(TARGET, env.get("smoke")).path
        print('Simulating', path)
        os.execv(path, [path])

    sim = Command('sim.txt', [], sim_run)
    Depends(sim, proj_elf(TARGET))
    Alias('sim', sim)

    sim_smoke = Command('sim_smoke.txt', [], sim_run, smoke=True)
    Depends(sim_smoke, proj_elf(TARGET, True))
    Alias('sim_smoke', sim_smoke)

    # open gdb with the elf file
    def gdb_run(target, source, env):
        path = proj_elf(TARGET, env.get("smoke")).path
        os.execv('/usr/bin/gdb', ['/usr/bin/gdb', path])

    gdb = Command('gdb.txt', [], gdb_run)
    Depends(gdb, proj_elf(TARGET))
    Alias('gdb', gdb)

    gdb_smoke = Command('gdb_smoke.txt', [], gdb_run, smoke=True)
    Depends(gdb_smoke, proj_elf(TARGET, True))
    Alias('gdb_smoke', gdb_smoke)

###########################################################
# Helper targets for arm
###########################################################
if PLATFORM == 'arm' and TYPE == 'project':
    # display memory info for the project
    if MEM_REPORT == 'true':
        get_mem_report = Action("python3 scons/mem_report.py " + "build/arm/bin/projects/{}".format(TARGET))
        env.AddPostAction(proj_bin(TARGET, False), get_mem_report)
        
    # flash the MCU using openocd
    def flash_run(target, source, env):
        OPENOCD = 'openocd'
        OPENOCD_SCRIPT_DIR = '/usr/share/openocd/scripts/'
        PROBE = 'cmsis-dap'
        OPENOCD_CFG = [
            OPENOCD,
            '-s {}'.format(OPENOCD_SCRIPT_DIR),
            '-f interface/{}.cfg'.format(PROBE),
            '-f target/stm32f1x.cfg',
            '-f {}/stm32f1-openocd.cfg'.format(PLATFORM_DIR),
            #'-c "stm32f1x.cpu configure -rtos FreeRTOS"',
            '-c "stm_flash {}"'.format(proj_bin(TARGET, env.get("smoke"))),
            '-c shutdown'
        ]
        cmd = 'sudo {}'.format(' '.join(OPENOCD_CFG))
        subprocess.run(cmd, shell=True)

    flash = Command('flash.txt', [], flash_run)
    Depends(flash, proj_bin(TARGET))
    Alias('flash', flash)

    flash_smoke = Command('flash_smoke.txt', [], flash_run, smoke=True)
    Depends(flash_smoke, proj_bin(TARGET, True))
    Alias('flash_smoke', flash_smoke)
