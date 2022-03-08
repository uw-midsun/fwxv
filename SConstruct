import json
import os
import sys
import subprocess
import glob
from scons.new_target import new_target

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

AddOption(
    '--define',
    dest='define',
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

env['CCCOMSTR'] = "Compiling $TARGET"
env['LINKCOMSTR'] = "Linking $TARGET"
env['ARCOMSTR'] = "Archiving $TARGET"
env['ASCOMSTR'] = "Assembling $TARGET"
env['RANLIBCOMSTR'] = "Indexing $TARGET"

###########################################################
# Directory setup
###########################################################

BUILD_DIR = Dir('build').Dir(PLATFORM)
BIN_DIR = BUILD_DIR.Dir('bin')
OBJ_DIR = BUILD_DIR.Dir('obj')
TEST_DIR = BUILD_DIR.Dir('test')

PROJ_DIR = Dir('projects')
LIB_DIR = Dir('libraries')
SMOKE_DIR = Dir('smoke')

PROJ_DIRS = [entry for entry in PROJ_DIR.glob('*')]
LIB_DIRS = [entry for entry in LIB_DIR.glob('*')]
SMOKE_DIRS = [entry for entry in SMOKE_DIR.glob('*')]

LIB_BIN_DIR = BIN_DIR.Dir('libraries')

PLATFORM_DIR = Dir('platform')

CODEGEN_DIR = LIB_DIR.Dir("codegen")
BOARDS_DIR = CODEGEN_DIR.Dir("boards")
GENERATOR = CODEGEN_DIR.File("generator.py")
TEMPLATES_DIR = CODEGEN_DIR.Dir("templates")
HEADER_OUTPUT_DIR = PROJ_DIR.Dir(PROJECT).Dir("inc")
LIBRARIES_INC_DIR = LIB_DIR.Dir("ms-common").Dir("inc")

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
def proj_elf(proj_name, is_smoke=False):
    return BIN_DIR.Dir(SMOKE_DIR.name if is_smoke else PROJ_DIR.name).File(proj_name)

# .bin is used for flashing to MCU
def proj_bin(proj_name, is_smoke=False):
    return proj_elf(proj_name, is_smoke).File(proj_name + '.bin')

###########################################################
# Header file generation from jinja templates
###########################################################
def generate_header_files(env, target, source):
    boards_dir = source[0]
    templates_dir = source[1]
    generator_dir = source[2]
    header_output_dir = source[3]
    libraries_inc_dir = source[4]
    templates = templates_dir.glob('*.jinja')
    base_exec = "python3 {} -b {}".format(generator_dir, PROJECT)
    header_files = []

    for template in templates:
        if template.name[0] == "_":
            header_files.append(header_output_dir.File(PROJECT + template.name[:-6]))
        else:
            header_files.append(header_output_dir.File(template.name[:-6]))

        if "can_board_ids" in template.name:
            env.Execute("{} -y {}.yaml -t {} -f {}".format(base_exec, boards_dir.File("boards"), template, libraries_inc_dir))
        else:
            if "_getters" in template.name:
                env.Execute("{} -y {}.yaml -t {} -f {}".format(base_exec, boards_dir.File(PROJECT), template, header_output_dir))
            else:
                env.Execute("{} -t {} -f {}".format(base_exec, template, header_output_dir))

    return header_files


# Create appropriate targets for all projects and libraries
for entry in PROJ_DIRS + LIB_DIRS + SMOKE_DIRS:
    # Glob the source files from OBJ_DIR because it's a variant dir
    # See: https://scons.org/doc/1.2.0/HTML/scons-user/x3346.html
    # str(entry) is e.g. 'projects/example', so this is like build/obj/projects/example/src
    srcs = OBJ_DIR.Dir(str(entry)).Dir('src').glob('*.[cs]')
    srcs += OBJ_DIR.Dir(str(entry)).Dir('src').Dir(PLATFORM).glob('*.[cs]')
    inc_dirs = [entry.Dir('inc')]
    inc_dirs += [entry.Dir('inc').Dir(PLATFORM)]

    config = parse_config(entry)

    # Just include all library headers
    # This resolves dependency issues like ms-freertos including FreeRTOS headers
    # even though FreeRTOS depends on ms-freertos, not the other way around
    lib_incs = [lib_dir.Dir('inc') for lib_dir in LIB_DIRS]
    lib_incs += [lib_dir.Dir('inc').Dir(PLATFORM) for lib_dir in LIB_DIRS]

    env.Append(CPPDEFINES=[GetOption('define')])

    # env.AddMethod(generate_header_files, "GenerateHeaderFiles")
    # header_targets = env.GenerateHeaderFiles(None, [BOARDS_DIR, TEMPLATES_DIR, GENERATOR, HEADER_OUTPUT_DIR, LIBRARIES_INC_DIR])

    if entry in PROJ_DIRS or entry in SMOKE_DIRS:
        is_smoke = entry in SMOKE_DIRS
        lib_deps = get_lib_deps(entry)
        output = proj_elf(entry.name, is_smoke)
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
            target = env.Bin(target=proj_bin(entry.name, is_smoke), source=target)
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
for entry in PROJ_DIRS + LIB_DIRS + SMOKE_DIRS:
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
            LINKFLAGS=env['LINKFLAGS'] + mock_link_flags,
        )
        if PLATFORM == 'arm':
            target = env.Bin(target=output.File(test_file.name + '.bin'), source=target)

        # Make test executable depend on the project / library final target
        if entry in PROJ_DIRS:
            Depends(target, proj_elf(entry.name, entry in SMOKE_DIRS))
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
    if entry and tests.get(entry):
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

def make_new_target(target, source, env):
    # No project or library option provided
    if not PROJECT and not LIBRARY:
        print("Missing project or library name. Expected --project=..., or --library=...")
        sys.exit(1)
    
    if str(target[0]) == 'new_smoke.txt' and PROJECT:
        target_type = 'smoke'
    elif PROJECT:
        target_type = 'project'
    elif LIBRARY:
        target_type = 'library'

    # Chain or's to select the first non-None value 
    new_target(target_type, PROJECT or LIBRARY or SMOKE)

new = Command('new_proj.txt', [], make_new_target)
Alias('new', new)
new = Command('new_smoke.txt', [], make_new_target)
Alias('new_smoke', new)

# 'clean.txt' is a dummy file that doesn't get created
# This is required for phony targets for scons to be happy
clean = Command('clean.txt', [], 'rm -rf build/*')
Alias('clean', clean)

###########################################################
# Linting and Formatting 
###########################################################

# Convert a list of paths/Dirs to space-separated paths.
def dirs_to_str(dir_list):
    # Use str(file) to ensure Dir objects are converted to paths.
    return ' '.join([str(file) for file in dir_list])

# Glob files by extension in a particular directory. Defaults to root directory.
def glob_by_extension(extension, dir='.'):
    return glob.glob('{}/**/*.{}'.format(str(dir), extension), recursive=True)

# Retrieve files to lint - returns a tuple (c_lint_files, py_lint_files)
def get_lint_files():
    c_lint_files = []
    py_lint_files = [] 

    lint_dirs = []

    # Get directories to lint based on PROJECT/LIBRARY args.
    # If no PROJECT/LIBRARY argument,lint all directories.
    if PROJECT:
        lint_dirs.append(PROJ_DIR.Dir(PROJECT))
        lint_dirs.append(SMOKE_DIR.Dir(PROJECT))
    elif LIBRARY:
        lint_dirs.append(LIB_DIR.Dir(LIBRARY))
    else:
        lint_dirs += PROJ_DIRS + LIB_DIRS

    # Get all src and header files (*.c, *.h) to lint/format
    for dir in lint_dirs: 
        c_files = glob_by_extension('[ch]', dir)
        py_files = glob_by_extension('py', dir) 

        config = parse_config(dir) 

        # Avoid linting/formatting external libraries
        if not config.get('no_lint'):
            c_lint_files += c_files 
            py_lint_files += py_files 

    return (c_lint_files, py_lint_files)

def run_lint(target, source, env):
    C_LINT_CMD = 'python ./scons/lint.py' 
    PY_LINT_CMD = 'pylint --rcfile={}/.pylintrc'.format(Dir('#').abspath) # '#' is the root dir

    c_lint_files, py_lint_files = get_lint_files()

    # Lint C source files
    if len(c_lint_files) > 0:
        print('\nLinting *.[ch] in {}, {} ...'.format(PROJ_DIR, LIB_DIR))
        subprocess.run('{} {}'.format(C_LINT_CMD, dirs_to_str(c_lint_files)), shell=True)

    # Lint Python files
    if len(py_lint_files) > 0:
        print('\nLinting *.py files ...')
        subprocess.run('{} {}'.format(PY_LINT_CMD, dirs_to_str(py_lint_files)), shell=True)

    print('Done Linting.')

def run_format(target, source, env):
    # Formatter configs
    AUTOPEP8_CONFIG = '-a --max-line-length 100 -r'
    CLANG_FORMAT_CONFIG = '-i -style=file'

    C_FORMAT_CMD = 'clang-format {}'.format(CLANG_FORMAT_CONFIG)
    PY_FORMAT_CMD = 'autopep8 {} -i'.format(AUTOPEP8_CONFIG)

    c_format_files, py_format_files = get_lint_files()

    # Format C source files
    if len(c_format_files) > 0:
        print('\nFormatting *.[ch] in {}, {} ...'.format(str(PROJ_DIR), str(LIB_DIR)))
        subprocess.run('{} {}'.format(C_FORMAT_CMD, dirs_to_str(c_format_files)), shell=True)

    # Format Python source files
    if len(py_format_files) > 0:
        print('\nFormatting *.py files ...')
        subprocess.run('{} {}'.format(PY_FORMAT_CMD, dirs_to_str(py_format_files)), shell=True)

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
        is_smoke = str(target[0]) == 'sim_smoke.txt'
        path = proj_elf(PROJECT, is_smoke).path
        print('Simulating', path)
        os.execv(path, [path])

    sim = Command('sim.txt', [], sim_run)
    Depends(sim, proj_elf(PROJECT))
    Alias('sim', sim)

    sim_smoke = Command('sim_smoke.txt', [], sim_run)
    Depends(sim_smoke, proj_elf(PROJECT, True))
    Alias('sim_smoke', sim_smoke)

    # open gdb with the elf file
    def gdb_run(target, source, env):
        is_smoke = str(target[0]) == 'gdb_smoke.txt'
        path = proj_elf(PROJECT, is_smoke).path
        os.execv('/usr/bin/gdb', ['/usr/bin/gdb', path])

    gdb = Command('gdb.txt', [], gdb_run)
    Depends(gdb, proj_elf(PROJECT))
    Alias('gdb', gdb)

    gdb_smoke = Command('gdb_smoke.txt', [], gdb_run)
    Depends(gdb_smoke, proj_elf(PROJECT, True))
    Alias('gdb_smoke', gdb_smoke)

###########################################################
# Helper targets for arm
###########################################################

if PLATFORM == 'arm' and PROJECT:
    # flash the MCU using openocd
    def flash_run(target, source, env):
        is_smoke = str(target[0]) == 'flash_smoke.txt'
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
            '-c "stm_flash {}"'.format(proj_bin(PROJECT, is_smoke)),
            '-c shutdown'
        ]
        cmd = 'sudo {}'.format(' '.join(OPENOCD_CFG))
        subprocess.run(cmd, shell=True)

    flash = Command('flash.txt', [], flash_run)
    Depends(flash, proj_bin(PROJECT))
    Alias('flash', flash)

    flash_smoke = Command('flash_smoke.txt', [], flash_run)
    Depends(flash_smoke, proj_bin(PROJECT, True))
    Alias('flash_smoke', flash_smoke)
