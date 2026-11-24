#!/usr/bin/env bash
# Boot an installed x86/x86_64 image and run the staged Android CTest suite.
set -euo pipefail

build_dir=$(realpath "${1:?Usage: android-run-emulator.sh <build-directory> <x86|x86_64>}")
abi=${2:?Specify x86 or x86_64}
case "$abi" in x86|x86_64) ;; *) echo "KVM tests require x86 or x86_64" >&2; exit 1 ;; esac
sdk=${ANDROID_HOME:?Set ANDROID_HOME to the Android SDK directory}
api=${ANDROID_API:?Set ANDROID_API}
export PATH="$sdk/platform-tools:$sdk/emulator:$sdk/cmdline-tools/latest/bin:$PATH"
export ANDROID_SERIAL=emulator-5554
export ANDROID_AVD_HOME="$build_dir/avd"
avd="fluidsynth-${abi}-$$"
logs="$build_dir/logs"
mkdir -p "$logs" "$ANDROID_AVD_HOME"
emulator_pid=

cleanup() {
    status=$?
    trap - EXIT
    if [[ -n "$emulator_pid" ]] && kill -0 "$emulator_pid" 2>/dev/null; then
        timeout 15 adb logcat -d > "$logs/logcat.txt" 2>&1 || true
        timeout 10 adb emu kill >/dev/null 2>&1 || true
        kill "$emulator_pid" 2>/dev/null || true
        wait "$emulator_pid" 2>/dev/null || true
    fi
    avdmanager delete avd -n "$avd" >/dev/null 2>&1 || true
    if (( status != 0 )) && [[ -f "$logs/emulator.log" ]]; then
        tail -100 "$logs/emulator.log" || true
    fi
    exit "$status"
}
trap cleanup EXIT

adb start-server
if adb devices | grep -q "^${ANDROID_SERIAL}[[:space:]]"; then
    echo "${ANDROID_SERIAL} is already in use" >&2
    exit 1
fi
avdmanager create avd --name "$avd" --package "system-images;android-$api;default;$abi" --path "$ANDROID_AVD_HOME/$avd.avd" --force <<< no
# Shrink the default ~7 GB userdata partition so the emulator fits on CI disks.
echo "disk.dataPartition.size=2048M" >> "$ANDROID_AVD_HOME/$avd.avd/config.ini"
avdmanager list avd > "$logs/avd.txt"
emulator -list-avds | tee "$logs/emulator-avds.txt" | grep -Fx "$avd"
emulator -avd "$avd" -port 5554 -no-window -no-snapshot -no-audio -no-boot-anim \
    -accel on -gpu swiftshader_indirect -cores 2 -memory 2048 -no-metrics > "$logs/emulator.log" 2>&1 &
emulator_pid=$!
boot_deadline=$((SECONDS + 300))
until [[ $(timeout 10 adb shell getprop sys.boot_completed 2>/dev/null | tr -d '\r') == 1 ]]; do
    if ! kill -0 "$emulator_pid" 2>/dev/null || (( SECONDS >= boot_deadline )); then
        echo 'Emulator did not finish booting within 300 seconds' >&2
        exit 1
    fi
    sleep 2
done

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
timeout 600 bash "$script_dir/../../test/run-android-tests.sh" "$build_dir/tests" 2>&1 | tee "$logs/native-tests.log"
