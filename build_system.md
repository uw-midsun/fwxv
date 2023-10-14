# Build System Documentation

We use scons for our build system. We chose scons because it's written in python which is easier to understand than GNU make, and it's what Tesla uses for their build system.

## Structure
Source files are split up into a number of projects and libraries. The gist of how things work is that when you run `scons`, it first iterates over all projects and libraries to understand what can be built and generating the dependency tree. Then, it picks what to build based on your command arguments, and then builds it and its dependencies. All build artifacts are placed in the `build` directory.

Each project and library may also contain a `config.json` file with certain fields. This is used to control the way the project or library is built, like including various library dependencies.

Unit tests are per-project/library and are built and run from scons. Functions may be mocked by specifying which test file mocks which functions. An example of a mocking configuration is in the `core` library, in `config.json` and in `test/test_mock.c`.

## Usage
### Build commands
- `no command`: Build all projects.
    - e.g. `scons`
- `project / library`: Build the project or library
    - e.g. `scons leds`
- `test`: (x86 specific) Builds and runs the tests for the specified project or library. If no project or library is specified, runs all tests. Also allows specifying a test file to run.
    - e.g. `scons test --platform=x86`
    - e.g. `scons test --platform=x86 --library=core`
    - e.g. `scons test --platform=x86 --library=core --test=status`
    - e.g. `scons test --platform=x86 --project=leds`

### Convenience commands:

- `new [name]`: Creates a new project, smoke project, or library with the given name.
    - e.g. `scons new --project=my_new_project`
    - e.g. `scons new --library=my_new_library`
    - e.g. `scons new_smoke --project=my_smoke_test`
- `new_task [name] [task name]`: Creates a new task within a project or library
    - e.g. `scons new_task --project=my_project --name=task_name`
    - e.g. `scons new_task --library=my_library --name=task_name`
- `clean`: Delete the `build` directory.
    - e.g. `scons clean`
- `sim [project]`: (x86 specific) Run the project's binary.
    - e.g. `scons sim --project=leds --platform=x86`
    - e.g. `scons sim_smoke --project=smoke_leds --platform=x86`
    - `sim_smoke` should be used for smoke tests.
- `gdb [project]`: (x86 specific) Run the project's binary with gdb.
    - e.g. `scons gdb --project=leds --platform=x86`
    - e.g. `scons gdb_smoke --project=smoke_leds --platform=x86`
- `flash [project]`: (arm specific) Flash the project's binary using openocd. A controller board must be connected an powered.
    - e.g. `scons flash --project=leds`
    - e.g. `scons flash_smoke --project=smoke_leds`
- `lint`: Lints all files.
    - Can specify `--project` or `--library` argument to only lint specific project/library
- `format`: Formats all files.
    - Can specify `--project` or `--library` argument to only format specific project/library

### Arguments
- `--platform`: Specifies target platform. One of `arm` or `x86`. Defaults to `arm`.
- `--project`: Specifies target project. Only required when running convenience commands or tests.
- `--library`: Specifies target library. Only required when running convenience commands or tests.
- `--test`: Specify test file to run.
- `--sanitizer`: Specifies the sanitizer. One of `asan` for Address sanitizer or `tsan` for Thread sanitizer. Defaults to `none`. Note it only works on `x86` platform.

# Future Improvements
Features that would be nice to have in the future but haven't been done yet:
- gdb on arm
- tests on arm
- split some code (e.g. code for building tests) into separate file
- move the config.json schema into its own file
- ensure unit test failures are handled properly
