import os

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
    '-std=c11',
    '-Wno-discarded-qualifiers',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
    '-Wsign-conversion',
    '-Wpointer-arith',
    '-Wno-enum-conversion',
    '-ffunction-sections',
    '-fdata-sections',
    '-Wl,--gc-sections',
    '-Wl,-Map=build/out.map',
    '--specs=nosys.specs',
    '--specs=nano.specs',
]

link_flags = [
    '-L{}/linker_scripts'.format(PLATFORM_DIR),
    '-Tstm32f1_default.ld',
]

arm_env = Environment(
    ENV = { 'PATH': os.environ['PATH'] },

    CC=compiler,
    CCFLAGS=cflags + arch_cflags + define_flags,
    CPPPATH=[],

    AS=compiler,
    ASFLAGS=['-c'] + cflags + arch_cflags + define_flags,
    
    LINK=compiler,
    LINKFLAGS=cflags + arch_cflags + link_flags,

    AR=ar,
    RANLIB=ranlib,

    LIBS=['m'],
)

bin_builder = Builder(action='{} -O binary $SOURCE $TARGET'.format(objcopy))
arm_env.Append(BUILDERS={'Bin': bin_builder})

Return('arm_env')
