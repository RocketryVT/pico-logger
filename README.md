# pico-logger
This library adds support for data logging on an RP2xxx device.

`cmake` and `arm-none-eabi-gcc` tooling required for successful build.

## Clone

**Note: You must initialize the git submodules prior to utilizing CMake for a proper build.**

```shell
git clone https://github.com/RocketryVT/pico-logger.git
cd pico-logger/
git submodule update --init --recursive
```

## Build Example (Linux)
```shell
cmake -B build -DCOMPILE_EXAMPLE=1
cmake --build build
```
In the event that your preferred IDE has trouble locating header files and/or is displaying incorrect errors, pass ```-DCMAKE_EXPORT_COMPILE_COMMANDS=true``` to the first CMake command above.

Binary files should be located in build/example/*.uf2 after a successful build.
