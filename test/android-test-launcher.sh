#!/usr/bin/env bash
# Execute one CTest command in the Android test directory, via ADB or a Bionic root.
set -euo pipefail
executable="./${1##*/}"
shift
command='cd /data/local/tmp/fluidsynth-tests && export LD_LIBRARY_PATH=/data/local/tmp/fluidsynth-tests/lib && exec'
for argument in "$executable" "$@"; do
    quoted=${argument//\'/\'\\\'\'}
    command+=" '$quoted'"
done
if [[ -n "${ANDROID_TEST_CHROOT:-}" ]]; then
    exec sudo chroot --userspec=2000:2000 "$ANDROID_TEST_CHROOT" /system/bin/sh -c "$command"
fi
exec adb shell "$command"
