# Android Testing Solution

This document describes the lightweight Android testing solution implemented for FluidSynth's CI pipeline.

## Problem Statement

The original Azure DevOps Android CI pipeline compiled FluidSynth for multiple Android architectures (armv7a, aarch64, x86, x86_64) but did not execute unit tests. The existing tests were designed for `ctest` and couldn't run directly on Android.

Previous attempts used a complex Android emulator + Gradle setup that was unreliable and heavy.

## Solution Overview

This solution enables Android unit test execution using **qemu-user-static** for cross-architecture emulation, integrated with the existing CMake build system.

### Key Benefits:
- ✅ **Lightweight**: No Android emulator required
- ✅ **Fast**: Direct execution via qemu-user-static  
- ✅ **Simple**: Uses existing `make check` and `ctest` infrastructure
- ✅ **Compatible**: Works with all Android architectures
- ✅ **Reliable**: No complex emulator setup that can fail
- ✅ **Non-Android-specific**: Uses standard Linux emulation tools

## Implementation Details

### 1. CMake Integration (`cmake_admin/FluidUnitTest.cmake`)

The `ADD_FLUID_TEST` macro was enhanced to automatically detect Android cross-compilation and configure appropriate qemu execution:

```cmake
# If cross-compiling for Android, use qemu-user to execute the test
if(ANDROID AND CMAKE_CROSSCOMPILING)
    # Determine the appropriate qemu binary based on target architecture
    if(ANDROID_ABI STREQUAL "armeabi-v7a")
        find_program(QEMU_BINARY qemu-arm-static)
    elseif(ANDROID_ABI STREQUAL "arm64-v8a")
        find_program(QEMU_BINARY qemu-aarch64-static)
    # ... etc for other architectures
    endif()
    
    if(QEMU_BINARY)
        ADD_TEST(NAME ${_test} COMMAND ${QEMU_BINARY} ${_test})
    endif()
endif()
```

### 2. Azure Pipeline Updates (`.azure/azure-pipelines-android.yml`)

#### Added qemu-user-static installation:
```yaml
- bash: |
    sudo -E apt-get -y install ... qemu-user-static
  displayName: 'apt-get install build-tools'
```

#### Replaced complex emulator setup with simple test execution:
```yaml
- bash: |
    # Set up library path for Android libraries
    export QEMU_LD_PREFIX="${PREFIX}"
    
    # Build and run tests
    pushd build_$(ANDROID_ABI_CMAKE)
    make -j$((`nproc`+1)) check
    popd
  displayName: 'Build and run unit tests with qemu'
```

### 3. Architecture Support

The solution automatically maps Android ABIs to appropriate qemu binaries:
- `armeabi-v7a` → `qemu-arm-static`
- `arm64-v8a` → `qemu-aarch64-static` 
- `x86` → `qemu-i386-static`
- `x86_64` → `qemu-x86_64-static`

## How It Works

1. **Build Phase**: FluidSynth and all dependencies are cross-compiled for Android as before
2. **Test Discovery**: CMake automatically detects Android cross-compilation and configures tests for qemu execution
3. **Test Execution**: `make check` builds tests and runs them via `ctest`
4. **Emulation**: qemu-user-static transparently executes Android binaries on the Linux CI host
5. **Library Resolution**: `QEMU_LD_PREFIX` environment variable points qemu to Android libraries

## Testing the Solution

To test locally (requires Android NDK):
```bash
# Configure for Android
cmake .. -DCMAKE_TOOLCHAIN_FILE=${NDK}/build/cmake/android.toolchain.cmake \
         -DANDROID_ABI=arm64-v8a -DANDROID_NATIVE_API_LEVEL=24

# Build and run tests  
make check
```

## Comparison with Previous Approach

| Aspect | Previous (Emulator + Gradle) | New (qemu-user-static) |
|--------|------------------------------|------------------------|
| **Complexity** | High (emulator setup, Gradle) | Low (single qemu install) |
| **Speed** | Slow (emulator boot time) | Fast (direct execution) |
| **Reliability** | Poor (emulator can fail) | High (simple emulation) |
| **Build System** | Custom Gradle setup | Native CMake integration |
| **Dependencies** | Android SDK, emulator images | qemu-user-static package |
| **Maintenance** | High (Android-specific) | Low (standard Linux tools) |

## Files Modified

- `cmake_admin/FluidUnitTest.cmake` - Added Android/qemu test execution logic
- `.azure/azure-pipelines-android.yml` - Simplified test execution, added qemu installation

## Future Enhancements

- Add fallback testing strategies if qemu fails
- Support for additional Android testing scenarios
- Performance optimizations for large test suites