import os
Import ('FLASH_TYPE', 'BUILD_TYPE')

PLATFORM_DIR = os.getcwd()

compiler = 'arm-none-eabi-gcc'
ranlib = 'arm-none-eabi-gcc-ranlib'
objcopy = 'arm-none-eabi-objcopy'
ar = 'arm-none-eabi-gcc-ar'

arch_cflags = [
    '-mlittle-endian',
    '-mcpu=cortex-m3',
    '-march=armv7-m',
    '-mthumb'
]

defines = [
    'MS_PLATFORM_ARM',
    'USE_STDPERIPH_DRIVER',
    'STM32F10X_MD', # Medium-density devices are STM32F103xx with 64-128k flash
    'HSE_VALUE=32000000',
]
define_flags = ['-D{}'.format(define) for define in defines]

cflags = [
    '-ffreestanding',
    '-Wall', # Enables all warnings
    '-Wextra', # Enables extra warnings
    '-Wno-discarded-qualifiers', # Disables warnings for qualifiers that are ignored
    '-Wno-unused-variable', # Disables warnings for unused variables
    '-Wno-unused-parameter',
    '-Wsign-conversion',
    '-Wpointer-arith',
    '-Wundef', # Warns if an undefined identifier is evaluated in an #if directive
    '-Wno-enum-conversion',
    '-ffunction-sections', # Places each function in its own section
    '-fdata-sections', # Places each data item in its own section
    '-fsingle-precision-constant', # Treats floating-point constants as single precision
    '-fno-math-errno', # Disables setting errno after math library functions
    '-Wl,--gc-sections', # Removes unused sections
    '-Wl,-Map=build/out.map',
    '--specs=nosys.specs',
    '--specs=nano.specs',
    '-fno-common', # Treats global variables as defined symbols
]

debug_cflags = [
    "-g3", # Generate extensive debugging information
    "-Og", # Optimizes code for debugging
    '-Werror',  # Treats all warnings as errors
    "-fstack-usage", # Generates stack usage information in .su files
    "-fstack-protector-strong", # Enables stack overflow protection
    
]
release_cflags = [
    "-Os", # Optimize for size
    '-flto', # Enables link-time optimization
    "-DNDEBUG", # Disables assertions
    "-fmerge-all-constants", # Merges identical constants
]

def get_link_flags(flash_type='default'):
    linker_scripts = {
        'default': 'stm32f1_default.ld',
        'bootloader': 'stm32f1_bootloader.ld',
        'application': 'stm32f1_application.ld'
    }
    script = linker_scripts.get(flash_type, linker_scripts['default'])
    return [
        '-L{}/linker_scripts'.format(PLATFORM_DIR),
        '-T{}'.format(script),
    ]

def create_arm_env(flash_type='default', build_type='debug'):
    combined_cflags = cflags + arch_cflags + define_flags
    if build_type == 'debug':
        combined_cflags += debug_cflags
    else:
        combined_cflags += release_cflags
    return Environment(
        ENV = { 'PATH': os.environ['PATH'] },

        CC=compiler,
        CCFLAGS=combined_cflags,
        CPPPATH=[],

        AS=compiler,
        ASFLAGS=['-c'] + combined_cflags,
        
        LINK=compiler,
        LINKFLAGS= combined_cflags + get_link_flags(flash_type),

        AR=ar,
        RANLIB=ranlib,

        LIBS=['m'],
    )

bin_builder = Builder(action='{} -O binary $SOURCE $TARGET'.format(objcopy))
arm_env = create_arm_env(FLASH_TYPE, BUILD_TYPE)
arm_env.Append(BUILDERS={'Bin': bin_builder})

Return('arm_env')
