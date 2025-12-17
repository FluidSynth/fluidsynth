#!/bin/bash

# Framework build script for iOS
# Assumes the build script is run on a Mac with XCode installed
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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
    -Denable-coreaudio=ON
    -Denable-sdl3=OFF
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
