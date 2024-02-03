# Build System Documentation

We use scons for our build system. We chose scons because it's written in python which is easier to understand than GNU make, and it's what Tesla uses for their build system.

## Structure
Source files are split up into a number of projects and libraries. The gist of how things work is that when you run `scons`, it first iterates over all projects and libraries to understand what can be built and generating the dependency tree. Then, it picks what to build based on your command arguments, and then builds it and its dependencies. All build artifacts are placed in the `build` directory.

Each project and library may also contain a `config.json` file with certain fields. This is used to control the way the project or library is built, like including various library dependencies.

Unit tests are per-project/library and are built and run from scons. Functions may be mocked by specifying which test file mocks which functions. An example of a mocking configuration is in the `core` library, in `config.json` and in `test/test_mock.c`.

## Usage
```
scons [options]... <command> <target>

Options:
options can occur anywhere in the command string

    --platform={x86|arm}    
        Specifies target platform. One of `arm` or `x86`. Defaults to `arm` if not provided.

    --define=...
        Add CPP defines to a build.
        - e.g.`--define="LOG_LEVEL=LOG_LEVEL_WARN"`

    --sanitizer={asan|tsan}
        (x86) Specifies the sanitizer. One of `asan` for Address sanitizer or `tsan` for Thread sanitizer. Defaults to `none`.
    
    --test=<test_name>
        Additionally specify the name of test to run for `test` command.

    --task=<task_name>
        Specify a task to create for `new` command.

    --mem-report
        (arm) Reports the memory space after a build

Commands:
    NONE
        Build the specified target, or all target if not specified.
        - e.g. `scons`
        - e.g. `scons <target>` (`scons --project=leds`)
        if the target is a python project, run the project.

    test
        Test the specified target, or all target if not specified.
        - e.g. `scons test`
        - e.g. `scons test <target>` (`scons test --project=leds`)
    
    new
        Creates a new project, smoke project, or library with the given name.
        - e.g. `scons new <target>` (`scons new --project=new_led`)
        if --task=<task> option is specified, instead create a new task within the target
        - e.g. `scons new <target> --task=<task_name>` (`scons new --project=leds --task=led_task`)

    sim
        (x86) Run the project's binary.
        - e.g. `scons sim --platform=x86 <target>` (`scons sim --platform=x86 --project=new_led`)

    gdb
        (x86) Run the project's binary with gdb.
        - e.g. `scons gdb <target>` (`scons gdb --project=new_led`)

    flash
        (arm) Flash the project's binary using openocd. A controller board must be connected an powered.
        - e.g. `scons flash <target>` (`scons flash --project=new_led`)

    format
        Format a target, or all targets if not specified. uses autopep8 for python and clang-format for c.
        - e.g. `scons format`
        - e.g. `scons format <target>` (`scons format --project=new_led`)

    lint
        Lint a target, or all targets if not specified. uses pylint for python and cpplint for c.
        - e.g. `scons lint`
        - e.g. `scons lint <target>` (`scons lint --project=new_led`)

    clean
        Delete the `build` directory.

Targets:
targets can be specified with an option.

    --project=<name>
        specify the target as a project with `name`
        - e.g. `--project=leds`

    --library=<name>
        specify the target as a library with `name`
        - e.g. `--library=ms-common`

    --python=<name>` or --py=<name>
        specify the target as a project with `name` (same as `--py=...`)
        - e.g. `--python=example`, `--py=example`

    --smoke=<name>`
        specify the target as a project with `name`, 
        - e.g. `--smoke=adc`

Convenience Commands:
    <name>
        build the target with the name
        - e.g. `scons leds`
        - e.g. `scons ms-common`
    
    <path>
        build the target with the path. 
        - e.g. `scons projects/leds`
        - e.g. `scons libraries/ms-common`

    test <name>/<path>
        test the target with the name/path
        - e.g. `scons test projects/leds`
        (experimental, use at own risk)
```

# Future Improvements
Features that would be nice to have in the future but haven't been done yet:
- gdb on arm
- move the config.json schema into its own file
