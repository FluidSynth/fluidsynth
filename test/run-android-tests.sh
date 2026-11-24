#!/usr/bin/env bash
# Run a staged Android CTest suite on an ADB device or a prepared Bionic root.
set -euo pipefail
tests=$(realpath "${1:?Usage: run-android-tests.sh <staged-test-directory> [ctest-args...]}")
shift
test -f "$tests/CTestTestfile.cmake"
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
export PATH="$script_dir:$PATH"
if [[ -z "${ANDROID_TEST_CHROOT:-}" ]]; then
    timeout 60 adb wait-for-device
    adb shell rm -rf /data/local/tmp/fluidsynth-tests
    adb shell mkdir -p /data/local/tmp/fluidsynth-tests
    adb push "$tests/." /data/local/tmp/fluidsynth-tests/
fi
# The tests share output filenames, so retain serial execution.
exec ctest --test-dir "$tests" --parallel 1 --timeout 120 --no-tests=error \
    --output-on-failure --output-junit "$tests/android-tests.xml" "$@"
