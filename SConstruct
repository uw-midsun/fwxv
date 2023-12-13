import os
from scons.common import flash_run

###########################################################
# Build arguments
###########################################################

AddOption(
    '--platform',
    dest='platform',
    type='choice',
    choices=("arm","x86"),
    default='arm'
)

AddOption(
    '--project',
    dest='project',
    type='string',
    action='store',
)

AddOption(
    '--smoke',
    dest='smoke',
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
    '--task',
    dest='task',
    type='string',
    action='store'
)

# Adding Memory Report Argument to Environment Flags
# Note platform needs to be explicitly set to arm

AddOption(
    '--mem-report',
    dest='mem-report',
    action='store_true',
)


AddOption(
    '--sanitizer',
    dest='sanitizer',
    type='choice',
    choices=("asan","tsan"),
    action='store',
    default="none"
)

PLATFORM = GetOption('platform')
PROJECT = GetOption('project')
SMOKE = GetOption('smoke')
LIBRARY = GetOption('library')
PYTHON = GetOption('python')
MEM_REPORT = GetOption('mem-report')

###########################################################
# Environment setup
###########################################################

# Retrieve the construction environment from the appropriate platform script
if PLATFORM == 'x86':
    env = SConscript('platform/x86.py')
elif PLATFORM == 'arm':
    env = SConscript('platform/arm.py')

TYPE = None
if PROJECT:
    TARGET = f'projects/{PROJECT}'
elif SMOKE:
    TARGET = f'smoke/{SMOKE}'
elif LIBRARY:
    TARGET = f'libraries/{LIBRARY}'
elif PYTHON:
    TARGET = f'py/{PYTHON}'
else:
    TARGET = None

print(f"target: {TARGET}")

VARS = {
    "PLATFORM": PLATFORM,
    "TARGET": TARGET,
    "env": env,
}
COMMAND = COMMAND_LINE_TARGETS[0] if COMMAND_LINE_TARGETS else ""

env["VARS"] = VARS

# Parse asan / tsan and Adding Sanitizer Argument to Environment Flags
# Note platform needs to be explicitly set to x86

SANITIZER = GetOption('sanitizer')

if SANITIZER == 'asan':
    env['CCFLAGS'] += ["-fsanitize=address"]
    env['CXXFLAGS'] += ["-fsanitize=address"]
    env['LINKFLAGS'] += ["-fsanitize=address"]
elif SANITIZER == 'tsan':
    env['CCFLAGS'] += ["-fsanitize=thread"]
    env['CXXFLAGS'] += ["-fsanitize=thread"]
    env['LINKFLAGS'] += ["-fsanitize=thread"]

env['CCCOMSTR'] = "Compiling  $TARGET"
env['LINKCOMSTR'] = "Linking    $TARGET"
env['ARCOMSTR'] = "Archiving  $TARGET"
env['ASCOMSTR'] = "Assembling $TARGET"
env['RANLIBCOMSTR'] = "Indexing   $TARGET"

env.Append(CPPDEFINES=[GetOption('define')])

###########################################################
# Directory setup
###########################################################

BUILD_DIR = Dir('#/build').Dir(PLATFORM)
BIN_DIR = BUILD_DIR.Dir('bin')
OBJ_DIR = BUILD_DIR.Dir('obj')

VariantDir(OBJ_DIR, '.', duplicate=0)

###########################################################
# Testing
###########################################################
if COMMAND.startswith("test"):
    # Add flags when compiling a test
    TEST_CFLAGS = ['-DMS_TEST=1']
    env['CCFLAGS'] += TEST_CFLAGS
    SConscript('scons/test.scons', exports='VARS')
    SConscript('scons/build.scons', exports='VARS')

###########################################################
# Helper targets
###########################################################
elif COMMAND == "new":
    SConscript('scons/new_target.scons', exports='VARS')

###########################################################
# Clean
###########################################################
elif COMMAND == "clean":
    # 'clean' is a dummy file that doesn't get created
    # This is required for phony targets for scons to be happy
    Command('#/clean', [], 'rm -rf build/*')
    # Alias('clean', clean)

###########################################################
# Linting and Formatting
###########################################################
elif COMMAND == "lint" or COMMAND == "format":
    SConscript('scons/lint_format.scons', exports='VARS')

###########################################################
# Build
###########################################################
else:  # command not recognised, default to build
    SConscript('scons/build.scons', exports='VARS')

###########################################################
# Helper targets for x86
###########################################################
if PLATFORM == 'x86' and TARGET:
    project_elf = BIN_DIR.File(TARGET)
    # os.exec the x86 project ELF file to simulate it

    def sim_run(target, source, env):
        path = project_elf
        print('Simulating', path)
        os.execv(path, [path])

    sim = Command('sim.txt', [], sim_run)
    Depends(sim, project_elf)
    Alias('sim', sim)

    # open gdb with the elf file
    def gdb_run(target, source, env):
        os.execv('/usr/bin/gdb', ['/usr/bin/gdb', project_elf.path])

    gdb = Command('gdb.txt', [], gdb_run)
    Depends(gdb, project_elf)
    Alias('gdb', gdb)

###########################################################
# Helper targets for arm
###########################################################
if PLATFORM == 'arm' and TARGET:
    project_bin = BIN_DIR.File(TARGET + '.bin')
    # display memory info for the project
    if MEM_REPORT:
        get_mem_report = Action(
            f"python3 scons/mem_report.py build/arm/bin/{TARGET}")
        env.AddPostAction(project_bin, get_mem_report)

    # flash the MCU using openocd
    def flash_run_target(target, source, env):
        serialData = flash_run(project_bin)
        while True:
            line: str = serialData.readline().decode("utf-8")
            print(line, end='')

    flash = Command('flash.txt', [], flash_run_target)
    Depends(flash, project_bin)
    Alias('flash', flash)
