import os

cflags = [
    '-g',
    '-Os',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-std=gnu11',
    '-Wno-discarded-qualifiers',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
    '-Wpointer-arith',
    '-ffunction-sections',
    '-fdata-sections',
    '-I/usr/local/include',
]

defines = [
    'MS_PLATFORM_X86',
    '_GNU_SOURCE',
]
define_flags = ['-D{}'.format(define) for define in defines]

link_flags = [
    '-lrt',
    '-lm',
    '-pthread',
]

x86_env = Environment(
    ENV={'PATH': os.environ['PATH']},

    CC='gcc',
    CCFLAGS=cflags + define_flags,
    CPPPATH=[],

    LINKFLAGS=link_flags,

    LIBS=[],

    CCCOMSTR="$TARGET",
    LINKCOMSTR="$TARGET"
)

Return('x86_env')
