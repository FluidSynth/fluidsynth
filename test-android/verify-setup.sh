#!/bin/bash

# Android Testing Verification Script
# This script helps verify that the Android emulator testing setup is working correctly

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
print_status "Checking prerequisites for Android testing..."

# Check if running from correct directory
if [ ! -f "CMakeLists.txt" ] || [ ! -d "test-android" ]; then
    print_error "Please run this script from the FluidSynth root directory"
    exit 1
fi

# Check for required tools
missing_tools=()

if ! command -v cmake &> /dev/null; then
    missing_tools+=("cmake")
fi

if ! command -v adb &> /dev/null; then
    missing_tools+=("adb")
fi

if [ ! -z "$ANDROID_NDK" ] && [ ! -d "$ANDROID_NDK" ]; then
    print_warning "ANDROID_NDK environment variable is set but directory doesn't exist: $ANDROID_NDK"
fi

if [ ${#missing_tools[@]} -gt 0 ]; then
    print_error "Missing required tools: ${missing_tools[*]}"
    print_error "Please install missing tools and make sure they're in PATH"
    exit 1
fi

print_status "All required tools found ✓"

# Check for Android emulator/device
print_status "Checking for Android device/emulator..."
if adb devices | grep -q "device$"; then
    device_count=$(adb devices | grep "device$" | wc -l)
    print_status "Found $device_count Android device(s) ✓"
    
    # Show device info
    adb devices | grep "device$" | while read line; do
        device_id=$(echo "$line" | cut -f1)
        device_abi=$(adb -s "$device_id" shell getprop ro.product.cpu.abi 2>/dev/null | tr -d '\r\n' || echo "unknown")
        print_status "  Device: $device_id (ABI: $device_abi)"
    done
else
    print_warning "No Android devices found"
    print_warning "Make sure an emulator is running or a device is connected"
    print_warning "You can start an emulator with:"
    print_warning "  \$ANDROID_HOME/emulator/emulator -avd your_avd_name"
fi

# Check Android NDK
print_status "Checking Android NDK..."
if [ -z "$ANDROID_NDK" ]; then
    print_warning "ANDROID_NDK environment variable not set"
    print_warning "You can set it to your NDK installation directory"
else
    if [ -d "$ANDROID_NDK" ]; then
        ndk_version=$(cat "$ANDROID_NDK/source.properties" 2>/dev/null | grep "Pkg.Revision" | cut -d'=' -f2 | tr -d ' ' || echo "unknown")
        print_status "Android NDK found: $ANDROID_NDK (version: $ndk_version) ✓"
    else
        print_warning "ANDROID_NDK points to non-existent directory: $ANDROID_NDK"
    fi
fi

# Check dependencies directory if available
if [ -d "android-build-root" ]; then
    print_status "Found android-build-root directory with dependencies ✓"
    lib_count=$(find android-build-root -name "*.so" 2>/dev/null | wc -l)
    print_status "  Found $lib_count shared libraries"
else
    print_warning "android-build-root directory not found"
    print_warning "You may need to build dependencies first"
fi

# Check if any build directories exist
build_dirs=$(find . -maxdepth 1 -name "build_*" -type d 2>/dev/null || true)
if [ ! -z "$build_dirs" ]; then
    print_status "Found existing build directories:"
    for dir in $build_dirs; do
        arch=$(basename "$dir" | sed 's/build_//')
        test_count=$(find "$dir" -name "*_android" -type f 2>/dev/null | wc -l)
        print_status "  $dir ($arch): $test_count test executables"
    done
else
    print_warning "No build directories found"
    print_warning "You'll need to build FluidSynth for Android first"
fi

print_status ""
print_status "Verification complete!"
print_status ""
print_status "Next steps to run Android tests:"
print_status "1. Build dependencies: cd test-android/build-scripts && ./download.sh && ./build-all-archs.sh"
print_status "2. Configure FluidSynth for Android using cmake with Android toolchain"
print_status "3. Build tests: make check-android"
print_status "4. Run tests: cd test-android && ./run-emulator-tests.sh"
print_status ""
print_status "For more details, see test-android/README_emulator_testing.md"