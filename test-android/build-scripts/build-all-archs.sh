#!/bin/bash

source ./build-env.sh

mkdir -p $DEV

# build

ANDROID_ABI_CMAKE=armeabi-v7a ./extract.sh || exit 1
ARCH='arm' ANDROID_ARCH='armv7a' ANDROID_ABI_CMAKE='armeabi-v7a' ANDROID_TARGET_ABI='eabi' AUTOTOOLS_TARGET="$ARCH-none-linux-android$ANDROID_TARGET_ABI" ./build.sh || exit 1

ANDROID_ABI_CMAKE=arm64-v8a ./extract.sh || exit 1
ARCH='aarch64' ANDROID_ARCH='aarch64' ANDROID_ABI_CMAKE='arm64-v8a' ANDROID_TARGET_ABI='' AUTOTOOLS_TARGET="$ARCH-none-linux-android" ./build.sh || exit 1

ANDROID_ABI_CMAKE=x86 ./extract.sh || exit 1
ARCH='i686' ANDROID_ARCH='i686' ANDROID_ABI_CMAKE='x86' ANDROID_TARGET_ABI='' AUTOTOOLS_TARGET="$ARCH-pc-linux-android" ./build.sh || exit 1

ANDROID_ABI_CMAKE=x86_64 ./extract.sh || exit 1
ARCH='x86_64' ANDROID_ARCH='x86_64' ANDROID_ABI_CMAKE='x86_64' ANDROID_TARGET_ABI='' AUTOTOOLS_TARGET="$ARCH-pc-linux-android" ./build.sh || exit 1
