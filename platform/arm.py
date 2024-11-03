import os
Import ('FLASH_TYPE')

PLATFORM_DIR = os.getcwd()

compiler = 'arm-none-eabi-gcc'
ranlib = 'arm-none-eabi-ranlib'
objcopy = 'arm-none-eabi-objcopy'
ar = 'arm-none-eabi-ar'

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
    '-Wall',
    '-Wextra',
    '-Werror',
    '-g3',
    '-Os',
    '-Wno-discarded-qualifiers',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
    '-Wsign-conversion',
    '-Wpointer-arith',
    '-Wundef',
    '-Wno-enum-conversion',
    '-ffunction-sections',
    '-fdata-sections',
    '-flto',
    '-fsingle-precision-constant',
    '-fno-math-errno',
    '-Wl,--gc-sections',
    '-Wl,-Map=build/out.map',
    '--specs=nosys.specs',
    '--specs=nano.specs',
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

def create_arm_env(flash_type='default'):
    return Environment(
        ENV = { 'PATH': os.environ['PATH'] },

        CC=compiler,
        CCFLAGS=cflags + arch_cflags + define_flags,
        CPPPATH=[],

        AS=compiler,
        ASFLAGS=['-c'] + cflags + arch_cflags + define_flags,
        
        LINK=compiler,
        LINKFLAGS=cflags + arch_cflags + get_link_flags(flash_type),

        AR=ar,
        RANLIB=ranlib,

        LIBS=['m'],
    )

bin_builder = Builder(action='{} -O binary $SOURCE $TARGET'.format(objcopy))
arm_env = create_arm_env(FLASH_TYPE)
arm_env.Append(BUILDERS={'Bin': bin_builder})

Return('arm_env')
