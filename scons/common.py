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
