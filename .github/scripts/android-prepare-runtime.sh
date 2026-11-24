#!/usr/bin/env bash
# Install an SDK system image and extract its Bionic userspace for ARM test jobs.
set -euo pipefail
api=${ANDROID_API:?Set ANDROID_API}
abi=${1:?Usage: android-prepare-runtime.sh <armeabi-v7a|arm64-v8a> <runtime-directory>}
runtime=${2:?Specify a runtime directory}
sdk=${ANDROID_HOME:?Set ANDROID_HOME to the Android SDK directory}
case "$abi" in
    armeabi-v7a) libdir=lib; linker=linker ;;
    arm64-v8a) libdir=lib64; linker=linker64 ;;
    *) echo "Unsupported native Android ABI: $abi" >&2; exit 1 ;;
esac
image="$sdk/system-images/android-$api/default/$abi"
if [[ ! -f "$image/system.img" || ! -f "$image/source.properties" ]]; then
    "$sdk/cmdline-tools/latest/bin/sdkmanager" "system-images;android-$api;default;$abi"
fi
mkdir -p "$runtime"
runtime=$(realpath "$runtime")
# Copy the Bionic userspace directly from the SDK's ext4 image.
export PATH="$PATH:/usr/sbin:/sbin"
rm -rf "$runtime/system" "$runtime/provenance.txt"
mkdir -p "$runtime/system"
for directory in bin "$libdir" etc usr; do
    debugfs -R "rdump /$directory $runtime/system" "$image/system.img"
done
debugfs -R "dump /build.prop $runtime/system/build.prop" "$image/system.img"
# debugfs can return success after a failed command. Check the runtime entry
# points and libraries before the build job saves or bundles this directory.
for executable in sh "$linker"; do
    if [[ ! -x "$runtime/system/bin/$executable" ]]; then
        echo "Missing Android runtime executable: $executable" >&2
        exit 1
    fi
done
for library in libc.so libm.so libdl.so; do
    if [[ ! -s "$runtime/system/$libdir/$library" ]]; then
        echo "Missing or empty Android runtime library: $libdir/$library" >&2
        exit 1
    fi
done
grep -E '^ro\.(build.version.sdk|product.cpu.abi)=' "$runtime/system/build.prop"
grep -qx "ro.build.version.sdk=$api" "$runtime/system/build.prop"
grep -qx "ro.product.cpu.abi=$abi" "$runtime/system/build.prop"
cp "$image/source.properties" "$runtime/provenance.txt"
