import json

def parse_config(entry):
    # Default config to empty for fields that don't exist
    ret = {
        'libs': [],
        'x86_libs': [],
        'arm_libs': [],
        'cflags': [],
        'mocks': {},
        'no_lint': False,
        "can": False,
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