import json
import os
import subprocess
import glob

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
    '--test',
    dest='testfile',
    type='string',
    action='store'
)

PLATFORM = GetOption('platform')
PROJECT = GetOption('project')
LIBRARY = GetOption('library')

###########################################################
# Environment setup
###########################################################

# Retrieve the construction environment from the appropriate platform script
if PLATFORM == 'x86':
    env = SConscript('platform/x86.py')
elif PLATFORM == 'arm':
    env = SConscript('platform/arm.py')

###########################################################
# Directory setup
###########################################################

BUILD_DIR = Dir('build').Dir(PLATFORM)
BIN_DIR = BUILD_DIR.Dir('bin')
OBJ_DIR = BUILD_DIR.Dir('obj')
TEST_DIR = BUILD_DIR.Dir('test')

PROJ_DIR = Dir('projects')
LIB_DIR = Dir('libraries')

PROJ_DIRS = [entry for entry in PROJ_DIR.glob('*')]
LIB_DIRS = [entry for entry in LIB_DIR.glob('*')]

LIB_BIN_DIR = BIN_DIR.Dir('libraries')

PLATFORM_DIR = Dir('platform')

# Put object files in OBJ_DIR so they don't clog the source folders
VariantDir(OBJ_DIR, Dir('.'), duplicate=0)

###########################################################
# Targets
###########################################################

def parse_config(entry):
    # Default config to empty for fields that don't exist
    ret = {
        'libs': [],
        'x86_libs': [],
        'arm_libs': [],
        'cflags': [],
        'mocks': {},
        'no_lint': False,
        'no_format': False
    }
    config_file = entry.File('config.json')
    if not config_file.exists():
        return ret
    with open(config_file.abspath, 'r') as f:
        config = json.load(f)
        for key, value in config.items():
            ret[key] = value
    return ret

# Recursively get library dependencies for entry
def get_lib_deps(entry):
    config = parse_config(entry)
    deps = config['libs'] + config['{}_libs'.format(PLATFORM)]
    for dep in deps:
        deps += get_lib_deps(LIB_DIR.Dir(dep))
    return deps

def lib_bin(lib_name):
    return BIN_DIR.Dir(LIB_DIR.name).File('lib{}.a'.format(lib_name))

# ELFs are used for gdb and x86
def proj_elf(proj_name):
    return BIN_DIR.Dir(PROJ_DIR.name).File(proj_name)

# .bin is used for flashing to MCU
def proj_bin(proj_name):
    return proj_elf(proj_name).File(proj_name + '.bin')

# Get all src files for a project/library
def get_srcs(dir):
    # Glob the source files from OBJ_DIR because it's a variant dir
    # See: https://scons.org/doc/1.2.0/HTML/scons-user/x3346.html
    # str(dir) is e.g. 'projects/example', so this is like build/obj/projects/example/src
    srcs = OBJ_DIR.Dir(str(dir)).Dir('src').glob('*.[cs]')
    srcs += OBJ_DIR.Dir(str(dir)).Dir('src').Dir(PLATFORM).glob('*.[cs]')
    return srcs

# Get header files for a project/library
def get_inc_files(dir):
    incs = dir.Dir('inc').glob('*.h')
    incs += dir.Dir('inc').Dir(PLATFORM).glob('*.h')
    return incs

# Get header directories for a project/library
def get_inc_dirs(dir):
    inc_dirs = [dir.Dir('inc')]
    inc_dirs += [dir.Dir('inc').Dir(PLATFORM)]
    return inc_dirs

# Get Python scripts for a project/library
def get_py_files(dir):
    # Assumes all Python files live in /scripts
    return dir.Dir('scripts').glob('*.py')


# Create appropriate targets for all projects and libraries
for entry in PROJ_DIRS + LIB_DIRS:
    srcs = get_srcs(entry)
    inc_dirs = get_inc_dirs(entry)

    config = parse_config(entry)

    # Just include all library headers
    # This resolves dependency issues like ms-freertos including FreeRTOS headers
    # even though FreeRTOS depends on ms-freertos, not the other way around
    lib_incs = [lib_dir.Dir('inc') for lib_dir in LIB_DIRS]
    lib_incs += [lib_dir.Dir('inc').Dir(PLATFORM) for lib_dir in LIB_DIRS]

    if entry in PROJ_DIRS:
        lib_deps = get_lib_deps(entry)
        output = proj_elf(entry.name)
        # SCons automagically handles object creation and linking
        target = env.Program(
            target=output,
            source=srcs,
            CPPPATH=env['CPPPATH'] + [inc_dirs, lib_incs],
            # link each library twice so that dependency cycles are resolved
            # See: https://stackoverflow.com/questions/45135
            LIBS=env['LIBS'] + lib_deps * 2,
            LIBPATH=[LIB_BIN_DIR],
            CCFLAGS=env['CCFLAGS'] + config['cflags'],
        )
        # .bin file only required for arm, not x86
        if PLATFORM == 'arm':
            target = env.Bin(target=proj_bin(entry.name), source=target)
    elif entry in LIB_DIRS:
        output = lib_bin(entry.name)
        target = env.Library(
            target=output,
            source=srcs,
            CPPPATH=env['CPPPATH'] + [inc_dirs, lib_incs],
            CCFLAGS=env['CCFLAGS'] + config['cflags'],
        )

    # Create an alias for the entry so we can do `scons leds` and it Just Works
    Alias(entry.name, target)

# Build all projects when you just run `scons`
Default([proj.name for proj in PROJ_DIRS])

###########################################################
# Testing
###########################################################

GEN_RUNNER = 'libraries/unity/auto/generate_test_runner.rb'

# tests dict maps proj/lib -> list of their test executables
tests = {}

# Create the test executable targets
for entry in PROJ_DIRS + LIB_DIRS:
    tests[entry.name] = []
    for test_file in OBJ_DIR.Dir(str(entry)).Dir('test').glob('*.c'):
        # Create the test_*_runner.c file
        runner_file = TEST_DIR.Dir(entry.name).File(test_file.name.replace('.c', '_runner.c'))
        test_runner = Command(runner_file, test_file, 'ruby {} $SOURCE $TARGET'.format(GEN_RUNNER))

        # Link runner object, test file object, and proj/lib objects
        # into executable
        config = parse_config(entry)
        test_module_name = test_file.name.replace('test_', '').replace('.c', '')
        mock_link_flags = []
        if test_module_name in config['mocks']:
            mocks = config['mocks'][test_module_name]
            mock_link_flags = ['-Wl,-wrap,' + mock for mock in mocks]

        objs = OBJ_DIR.Dir(str(entry)).Dir('src').glob('*.o')
        objs += OBJ_DIR.Dir(str(entry)).Dir('src').Dir(PLATFORM).glob('*.o')
        entry_objects = []
        for obj in objs:
            if 'main.o' not in obj.name:
                entry_objects.append(obj)

        inc_dirs = [entry.Dir('inc')]
        inc_dirs += [entry.Dir('inc').Dir(PLATFORM)]
        lib_incs = [lib_dir.Dir('inc') for lib_dir in LIB_DIRS]
        lib_incs += [lib_dir.Dir('inc').Dir(PLATFORM) for lib_dir in LIB_DIRS]
        lib_deps = get_lib_deps(entry)

        output = TEST_DIR.Dir(entry.name).Dir('test').File(test_file.name.replace('.c', ''))
        target = env.Program(
            target=output,
            source=[test_file, test_runner] + entry_objects,
            # We do env['variable'] + [entry-specific variables] to avoid
            # mutating the environment for other entries
            CPPPATH=env['CPPPATH'] + [inc_dirs, lib_incs],
            LIBS=env['LIBS'] + lib_deps * 2 + ['unity'],
            LIBPATH=[LIB_BIN_DIR],
            CCFLAGS=env['CCFLAGS'] + config['cflags'],
            LINKFLAGS=mock_link_flags,
        )
        if PLATFORM == 'arm':
            target = env.Bin(target=output.File(test_file.name + '.bin'), source=target)

        # Make test executable depend on the project / library final target
        if entry in PROJ_DIRS:
            Depends(target, proj_elf(entry.name))
        elif entry in LIB_DIRS:
            Depends(target, lib_bin(entry.name))

        # Add to tests dict
        tests[entry.name] += [node for node in target]

def get_test_list():
    # Based on the project/library and test in options,
    # create a list of tests to run
    proj = PROJECT if PROJECT else ''
    lib = LIBRARY if LIBRARY else ''
    # Assume only one of project or library is set
    entry = proj + lib
    if entry:
        if GetOption('testfile'):
            return [test for test in tests[entry] if test.name == 'test_' + GetOption('testfile')]
        else:
            return [test for test in tests[entry]]
    else:
        ret = []
        for test_list in tests.values():
            ret += test_list
        return ret

def test_runner(target, source, env):
    test_list = get_test_list()
    for test in test_list:
        subprocess.run(test.get_path())

test = Command('test.txt', [], test_runner)
Depends(test, get_test_list())
Alias('test', test)

###########################################################
# Helper targets
###########################################################

# 'clean.txt' is a dummy file that doesn't get created
# This is required for phony targets for scons to be happy
clean = Command('clean.txt', [], 'rm -rf build/*')
Alias('clean', clean)

###########################################################
# Linting and Formatting 
###########################################################

lint_files = []
format_files = []
py_lint_files = [] 
py_format_files = [] 

# Python project directories (e.g /mpxe from firmware_xiv)
PYTHON_DIRS = []

# Formatter configs
AUTOPEP8_CONFIG = '-a --max-line-length 100 -r'
CLANG_FORMAT_CONFIG = '-i style=file'

def dir_list_to_str(dir_list):
    return ' '.join([str(file) for file in dir_list])


# Get all src and header files (*.c, *.h) to lint/format
for entry in PROJ_DIRS + LIB_DIRS:
    srcs = get_srcs(entry)
    incs = get_inc_files(entry)
    py_files = get_py_files(entry)

    config = parse_config(entry) 

    # Avoid linting/formatting external libraries
    if not config.get('no_format'):
        format_files += srcs + incs
        py_format_files += py_files
    if not config.get('no_lint'):
        lint_files += srcs + incs
        py_lint_files += py_files 

for entry in PYTHON_DIRS:
    # Use Python glob instead of Scons glob, recursive search needed.
    py_files = glob.glob('{}/**/*.py'.format(str(entry)), recursive=True)
    py_format_files += py_files
    py_lint_files += py_files

def run_lint(target, source, env):
    print('\nLinting *.[ch] in {}, {} ...'.format(PROJ_DIR, LIB_DIR))
    lint_cmd = 'echo {} | xargs -r python2 ./lint.py'.format(dir_list_to_str(lint_files))
    subprocess.run(lint_cmd, shell=True)

    print('\nLinting *.py files ...')
    pylint_cmd = 'echo {} | xargs -r pylint'.format(dir_list_to_str(py_lint_files))
    subprocess.run(pylint_cmd, shell=True)

def run_format(target, source, env):
    print('\nFormatting *.[ch] in {}, {} ...'.format(str(PROJ_DIR), str(LIB_DIR)))
    format_cmd = ('echo {} | xargs -r clang-format {}'
                    .format(dir_list_to_str(format_files), CLANG_FORMAT_CONFIG))
    subprocess.run(format_cmd, shell=True)

    print('\nFormatting *.py files ...')
    autopep8_cmd = ('autopep8 {1} -i {0}'
                    .format(dir_list_to_str(py_format_files), AUTOPEP8_CONFIG))
    subprocess.run(autopep8_cmd, shell=True)

    print('Done Formatting.')

lint = Command('lint.txt', [], run_lint)
Alias('lint', lint)

format = Command('format.txt', [], run_format)
Alias('format', format)

###########################################################
# Helper targets for x86
###########################################################

if PLATFORM == 'x86' and PROJECT:
    # os.exec the x86 project ELF file to simulate it
    def sim_run(target, source, env):
        path = proj_elf(PROJECT).path
        print('Simulating', PROJECT)
        os.execv(path, [path])

    sim = Command('sim.txt', [], sim_run)
    Depends(sim, proj_elf(PROJECT))
    Alias('sim', sim)

    # open gdb with the elf file
    def gdb_run(target, source, env):
        path = proj_elf(PROJECT).path
        os.execv('/usr/bin/gdb', ['/usr/bin/gdb', path])

    gdb = Command('gdb.txt', [], gdb_run)
    Depends(gdb, proj_elf(PROJECT))
    Alias('gdb', gdb)

###########################################################
# Helper targets for arm
###########################################################

if PLATFORM == 'arm' and PROJECT:
    # flash the MCU using openocd
    def flash_run(target, source, env):
        OPENOCD = 'openocd'
        OPENOCD_SCRIPT_DIR = '/usr/share/openocd/scripts/'
        PROBE = 'cmsis-dap'
        OPENOCD_CFG = [
            OPENOCD,
            '-s {}'.format(OPENOCD_SCRIPT_DIR),
            '-f interface/{}.cfg'.format(PROBE),
            '-f target/stm32f0x.cfg',
            '-f {}/stm32f0-openocd.cfg'.format(PLATFORM_DIR),
            '-c "stm32f0x.cpu configure -rtos FreeRTOS"',
            '-c "stm_flash {}"'.format(proj_bin(PROJECT)),
            '-c shutdown'
        ]
        cmd = 'sudo {}'.format(' '.join(OPENOCD_CFG))
        subprocess.run(cmd, shell=True)

    flash = Command('flash.txt', [], flash_run)
    Depends(flash, proj_bin(PROJECT))
    Alias('flash', flash)
