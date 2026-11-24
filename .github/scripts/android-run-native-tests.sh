#!/usr/bin/env bash
# Execute Android/Bionic binaries directly on the ARM host's Linux kernel.
# GitHub's hosted ARM runners do not expose nested KVM for an accelerated Android
# guest. Reuse their kernel to test both ARM ABIs without CPU emulation; this path
# does not exercise Android framework services or Android device drivers.
set -euo pipefail
abi=${1:?Usage: android-run-native-tests.sh <ABI> <test-bundle> <work-directory>}
bundle=$(realpath "${2:?Specify a test bundle}")
work=${3:?Specify a work directory}
test "$(uname -m)" = aarch64
case "$abi" in
    armeabi-v7a|arm64-v8a) ;;
    *) echo "Unsupported native Android ABI: $abi" >&2; exit 1 ;;
esac
if grep -l 'qemu-\(arm\|aarch64\)' /proc/sys/fs/binfmt_misc/* 2>/dev/null; then
    echo 'Disable QEMU binfmt handlers before checking native ARM execution.' >&2
    exit 1
fi
mkdir -p "$work"
work=$(realpath "$work")
tar -xzf "$bundle" -C "$work"
runtime="$work/runtime"
root="$work/root"
# Android reserves UID/GID 2000 for shell. Mount and ownership setup use sudo;
# execute tests as this unprivileged Android identity.
android_shell=2000:2000
mkdir -p "$root"/{system,proc,dev,data/local/tmp/fluidsynth-tests} "$work/logs"
cleanup() {
    local status=$?
    trap - EXIT
    for directory in dev proc system; do
        if mountpoint -q "$root/$directory"; then
            sudo umount "$root/$directory" || sudo umount -l "$root/$directory" || true
        fi
    done
    exit "$status"
}
trap cleanup EXIT
# Bind the Bionic userspace read-only; /proc and /dev expose host kernel views.
sudo mount --bind "$runtime/system" "$root/system"
sudo mount -o remount,bind,ro "$root/system"
sudo mount -t proc proc "$root/proc"
sudo mount --bind /dev "$root/dev"
cp -a "$work/tests/." "$root/data/local/tmp/fluidsynth-tests/"
sudo chown -R "$android_shell" "$root/data"
uname -a
lscpu
cat "$runtime/provenance.txt"
grep -E '^ro\.(build.version.sdk|product.cpu.abi)=' "$runtime/system/build.prop"
sdk_api=$(sed -n 's/^ro\.build\.version\.sdk=//p' "$runtime/system/build.prop")
printf 'Execution: native %s CPU, Android API %s Bionic, host Linux kernel\n' "$abi" "${sdk_api:-unknown}"
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ANDROID_TEST_CHROOT="$root" timeout 600 \
    bash "$script_dir/../../test/run-android-tests.sh" "$work/tests" 2>&1 \
    | tee "$work/logs/native-tests.log"
