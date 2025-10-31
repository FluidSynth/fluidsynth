#!/bin/bash

# Framework build script for iOS
# SDL3 is used instead of CoreAudio as CoreAudio/AudioHardware.h is not available in iOS.
# Assumes the built SDL3 framework for both iOS and iOS simulator is located in /path/to/fluidsynth/SDL/build/SDL3.xcframework 
# Also assumes the build script is run on a Mac with XCode installed
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL3_XCFRAMEWORK_DIR="${SCRIPT_DIR}/../SDL/build/SDL3.xcframework"
SDL3_CMAKE_DIR_IOS="${SDL3_XCFRAMEWORK_DIR}/ios-arm64/SDL3.framework/CMake"
SDL3_CMAKE_DIR_IOS_SIMULATOR="${SDL3_XCFRAMEWORK_DIR}/ios-arm64_x86_64-simulator/SDL3.framework/CMake"

# Clean up previous builds
rm -rf build
mkdir -p build
cd build
rm -rf build-ios build-ios-simulator
mkdir -p build-ios build-ios-simulator

# Common CMake flags across builds
CMAKE_COMMON_FLAGS=(
    -G Xcode
    -Dosal=cpp11
    -Denable-framework=ON
    -Denable-libinstpatch=0
    -Denable-aufile=OFF
    -Denable-dbus=OFF
    -Denable-ladspa=OFF
    -Denable-midishare=OFF
    -Denable-opensles=OFF
    -Denable-oboe=OFF
    -Denable-oss=OFF
    -Denable-pipewire=OFF
    -Denable-portaudio=OFF
    -Denable-pulseaudio=OFF
    -Denable-readline=OFF
    -Denable-coreaudio=OFF
    -Denable-sdl3=ON
    -Denable-systemd=OFF
    -Denable-threads=ON
    -Denable-waveout=OFF
    -Denable-network=OFF
    -Denable-ipv6=OFF
    -Denable-libsndfile=OFF
    -DCMAKE_MACOSX_BUNDLE=NO
    -DBUILD_SHARED_LIBS=ON
)

# Download iOS toolchain
curl -o ios.toolchain.cmake https://raw.githubusercontent.com/leetal/ios-cmake/master/ios.toolchain.cmake

# Build for iOS device (arm64)
echo "Building for iOS device (arm64)..."
cd build-ios
cmake ../.. \
    -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake \
    -DPLATFORM=OS64 \
    -DSDL3_DIR="${SDL3_CMAKE_DIR_IOS}" \
    "${CMAKE_COMMON_FLAGS[@]}"

xcodebuild -project FluidSynth.xcodeproj -target libfluidsynth -configuration Release -sdk iphoneos \
    build DEBUG_INFORMATION_FORMAT="dwarf-with-dsym" GCC_GENERATE_DEBUGGING_SYMBOLS=YES 2>&1
cd ..

# Build for iOS simulator
echo "Building for iOS simulator..."
cd build-ios-simulator
cmake ../../ \
    -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake \
    -DPLATFORM=SIMULATOR64COMBINED \
    -DSDL3_DIR="${SDL3_CMAKE_DIR_IOS_SIMULATOR}" \
    "${CMAKE_COMMON_FLAGS[@]}"

xcodebuild -project FluidSynth.xcodeproj -target libfluidsynth -configuration Release -sdk iphonesimulator \
    build DEBUG_INFORMATION_FORMAT="dwarf-with-dsym" GCC_GENERATE_DEBUGGING_SYMBOLS=YES 2>&1
cd ..

# Create XCFramework from the built dynamic frameworks
echo "Creating XCFramework..."
rm -rf FluidSynth.xcframework
BUILD_DIR_ABSOLUTE=$(pwd)
xcodebuild -create-xcframework \
    -framework "${BUILD_DIR_ABSOLUTE}/build-ios/src/Release-iphoneos/FluidSynth.framework" \
    -debug-symbols "${BUILD_DIR_ABSOLUTE}/build-ios/src/Release-iphoneos/FluidSynth.framework.dSYM" \
    -framework "${BUILD_DIR_ABSOLUTE}/build-ios-simulator/src/Release-iphonesimulator/FluidSynth.framework" \
    -debug-symbols "${BUILD_DIR_ABSOLUTE}/build-ios-simulator/src/Release-iphonesimulator/FluidSynth.framework.dSYM" \
    -output "${BUILD_DIR_ABSOLUTE}/FluidSynth.xcframework"

echo "FluidSynth.xcframework (dynamic framework) built successfully at build/FluidSynth.xcframework"
