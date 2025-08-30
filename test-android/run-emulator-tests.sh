#!/bin/bash

# Android Emulator Test Runner
# This script runs individual FluidSynth test executables in the Android emulator
# Replaces the Gradle-based approach with direct ADB execution

set -e

# Configuration
ANDROID_ABI_CMAKE="${ANDROID_ABI_CMAKE:-arm64-v8a}"
ANDROID_API="${ANDROID_API:-24}"
BUILD_DIR="${BUILD_DIR:-build_${ANDROID_ABI_CMAKE}}"
TEST_DIR="/data/local/tmp/fluidsynth_tests"
RESULTS_FILE="/data/local/tmp/test_results.txt"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if ADB is available
if ! command -v adb &> /dev/null; then
    print_error "ADB command not found. Please ensure Android SDK is installed and ADB is in PATH."
    exit 1
fi

# Wait for device
print_status "Waiting for Android device/emulator..."
adb wait-for-device

# Check if device is ready
if ! adb shell echo "Device ready" &> /dev/null; then
    print_error "Device not accessible via ADB"
    exit 1
fi

print_status "Device ready"

# Create test directory on device
print_status "Creating test directory on device: ${TEST_DIR}"
adb shell "mkdir -p ${TEST_DIR}" || true

# Find all test executables
TEST_EXECUTABLES=$(find "${BUILD_DIR}/test/android" -name "*_android" -type f 2>/dev/null || true)

if [ -z "$TEST_EXECUTABLES" ]; then
    print_error "No Android test executables found in ${BUILD_DIR}/test/android"
    print_error "Make sure to build the tests first with: make check-android"
    print_error "Available files in build dir:"
    find "${BUILD_DIR}" -name "*test*" -type f 2>/dev/null | head -10 || true
    exit 1
fi

print_status "Found $(echo "$TEST_EXECUTABLES" | wc -l) test executables"

# Push test executables to device
print_status "Pushing test executables to device..."
for test_exe in $TEST_EXECUTABLES; do
    test_name=$(basename "$test_exe")
    print_status "  Pushing $test_name"
    if ! adb push "$test_exe" "${TEST_DIR}/${test_name}"; then
        print_warning "Failed to push $test_name, skipping..."
        continue
    fi
    adb shell "chmod 755 ${TEST_DIR}/${test_name}"
done

# Also need to push required shared libraries if they exist
LIB_DIR="${PREFIX}/lib"
if [ -d "$LIB_DIR" ]; then
    print_status "Pushing shared libraries to device..."
    # Create lib directory on device
    adb shell "mkdir -p ${TEST_DIR}/lib" || true
    
    # Push essential libraries (only the ones that exist)
    for lib in libfluidsynth.so libglib-2.0.so libgobject-2.0.so libgio-2.0.so libgmodule-2.0.so libgthread-2.0.so libsndfile.so libinstpatch-1.0.so; do
        if [ -f "${LIB_DIR}/${lib}" ]; then
            print_status "  Pushing $lib"
            adb push "${LIB_DIR}/${lib}" "${TEST_DIR}/lib/" || print_warning "Failed to push $lib"
        fi
    done
fi

# Get device info for debugging
print_status "Device information:"
adb shell "uname -a" || true
adb shell "getprop ro.product.cpu.abi" || true

# Initialize results
adb shell "echo 'FluidSynth Test Results' > ${RESULTS_FILE}"
adb shell "echo '======================' >> ${RESULTS_FILE}"
adb shell "echo 'Date: '$(date) >> ${RESULTS_FILE}"
adb shell "echo 'Architecture: ${ANDROID_ABI_CMAKE}' >> ${RESULTS_FILE}"
adb shell "echo '' >> ${RESULTS_FILE}"

# Run tests
print_status "Running tests..."
total_tests=0
passed_tests=0
failed_tests=0

for test_exe in $TEST_EXECUTABLES; do
    test_name=$(basename "$test_exe")
    print_status "Running $test_name"
    
    total_tests=$((total_tests + 1))
    
    # Set library path and run test
    test_command="cd ${TEST_DIR} && LD_LIBRARY_PATH=${TEST_DIR}/lib:\$LD_LIBRARY_PATH ./${test_name}"
    
    # Run the test and capture both exit code and output
    if adb shell "$test_command" 2>&1; then
        test_exit_code=$?
        if [ $test_exit_code -eq 0 ]; then
            print_status "  ✓ $test_name PASSED"
            adb shell "echo '[PASS] $test_name' >> ${RESULTS_FILE}"
            passed_tests=$((passed_tests + 1))
        else
            print_error "  ✗ $test_name FAILED (exit code: $test_exit_code)"
            adb shell "echo '[FAIL] $test_name (exit code: $test_exit_code)' >> ${RESULTS_FILE}"
            failed_tests=$((failed_tests + 1))
        fi
    else
        print_error "  ✗ $test_name FAILED (could not execute)"
        adb shell "echo '[FAIL] $test_name (could not execute)' >> ${RESULTS_FILE}"
        failed_tests=$((failed_tests + 1))
    fi
done

# Write summary
adb shell "echo '' >> ${RESULTS_FILE}"
adb shell "echo 'Summary:' >> ${RESULTS_FILE}"
adb shell "echo 'Total tests: ${total_tests}' >> ${RESULTS_FILE}"
adb shell "echo 'Passed: ${passed_tests}' >> ${RESULTS_FILE}"
adb shell "echo 'Failed: ${failed_tests}' >> ${RESULTS_FILE}"

# Pull results file
print_status "Retrieving test results..."
adb pull "${RESULTS_FILE}" "./android_test_results.txt" || true

# Display summary
echo ""
print_status "Test Summary:"
echo "Total tests: $total_tests"
echo "Passed: $passed_tests"
echo "Failed: $failed_tests"

if [ $failed_tests -eq 0 ]; then
    print_status "All tests passed! ✓"
    exit 0
else
    print_error "$failed_tests test(s) failed!"
    exit 1
fi