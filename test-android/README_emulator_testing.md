# Android Emulator Testing Setup

This directory contains scripts and configurations for running FluidSynth unit tests directly in the Android emulator without requiring Gradle.

## Overview

The new approach replaces the previous Gradle-based testing system with individual test executables that run directly in the Android emulator via ADB. This provides several advantages:

1. **Maintainability**: Each test remains in its own C source file
2. **No Gradle dependency**: Tests can be built and run using only CMake and ADB
3. **Better debugging**: Individual test failures are easier to isolate
4. **Architecture support**: Works with armv7a, aarch64, x86, and x86_64

## Files

- `run-emulator-tests.sh`: Main script that pushes and executes test binaries in the emulator
- `convert-tests.sh`: Legacy script for Gradle-based approach (deprecated)
- `app/`: Legacy Android Studio project structure (deprecated)

## How it works

1. **Build Phase**: CMake builds individual test executables using the `ADD_FLUID_ANDROID_TEST` macro
2. **Deployment Phase**: The test runner script pushes test binaries and required libraries to the emulator
3. **Execution Phase**: Each test is executed individually via `adb shell`
4. **Reporting Phase**: Results are collected and reported back to the CI system

## Usage

### Local Testing

```bash
# Build FluidSynth for Android (assuming dependencies are available)
cmake -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_NATIVE_API_LEVEL=24 \
      ..
make

# Build test executables
make check-android

# Start emulator (or connect device)
$ANDROID_HOME/emulator/emulator -avd your_avd_name &

# Run tests
cd test-android
./run-emulator-tests.sh
```

### CI Integration

The Azure pipeline automatically:
1. Downloads and builds dependencies
2. Compiles FluidSynth for target architecture
3. Builds individual test executables
4. Creates and starts Android emulator
5. Executes tests and collects results

## Architecture

The system consists of:

- **FluidAndroidTest.cmake**: CMake macro for building Android test executables
- **Azure pipeline integration**: Modified azure-pipelines-android.yml
- **Test runner script**: Handles emulator interaction and result collection

## Migration from Gradle

The old Gradle-based approach:
- Converted all test `main()` functions to unique names
- Compiled everything into a single shared library
- Required Android Studio project structure

The new approach:
- Builds each test as a separate executable
- Links against libfluidsynth-OBJ and dependencies
- Executes directly in emulator via ADB

## Troubleshooting

### Test executables not found
- Ensure `make check-android` was run after building FluidSynth
- Check that `ANDROID` CMake variable is set during configuration

### Library not found errors
- Verify that required shared libraries are available in `${PREFIX}/lib`
- Check that `LD_LIBRARY_PATH` is correctly set in the emulator

### Emulator connection issues
- Ensure `adb` is in PATH and emulator is running
- Check that emulator has sufficient storage space

### Test failures
- Individual test logs are available in the emulator at `/data/local/tmp/test_results.txt`
- Use `adb logcat` for additional debugging information