#!/usr/bin/env bash
# Build FluidSynth and its Android dependencies, or stage shared libraries and tests.
set -Eeuo pipefail
current_phase=setup
trap 'status=$?; printf "Android %s failed (exit %s)\n" "$current_phase" "$status" >&2; exit "$status"' ERR
phase=${1:?Usage: android-build.sh <build-dependencies|build-and-install|package>}
case "$phase" in
    build-dependencies|build-and-install|package) ;;
    *) echo "Unknown Android build operation: $phase" >&2; exit 2 ;;
esac
export ANDROID_API=${ANDROID_API:-24}
api=$ANDROID_API
if [[ ! "$api" =~ ^[0-9]+$ ]] || (( api < 24 )); then
    echo 'Android builds require API 24 or newer.' >&2
    exit 2
fi
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
ndk=${ANDROID_NDK_HOME:?Set ANDROID_NDK_HOME}
if [[ ! -f "$ndk/build/cmake/android.toolchain.cmake" ]]; then
    printf 'Android NDK toolchain not found under %s\n' "$ndk" >&2
    exit 2
fi
abi=${ABI:?Set ABI}
case "$abi" in
    armeabi-v7a) triple=arm-linux-androideabi; triplet=arm-android-shared ;;
    arm64-v8a) triple=aarch64-linux-android; triplet=arm64-android-shared ;;
    x86) triple=i686-linux-android; triplet=x86-android-shared ;;
    x86_64) triple=x86_64-linux-android; triplet=x64-android-shared ;;
    *) echo "Unknown Android ABI: $abi" >&2; exit 1 ;;
esac
build_type=${BUILD_TYPE:-RelWithDebInfo}
work=${ANDROID_BUILD_DIR:-"$root/build/android/$abi-api$api-$build_type"}
mkdir -p "$work"
work=$(cd -- "$work" && pwd)
prefix="$work/dependencies/$triplet"
jobs=${CMAKE_BUILD_PARALLEL_LEVEL:-$(getconf _NPROCESSORS_ONLN)}
export PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR="$prefix/lib/pkgconfig"
args=(-G Ninja "-DCMAKE_TOOLCHAIN_FILE=$ndk/build/cmake/android.toolchain.cmake"
    "-DANDROID_ABI=$abi" "-DANDROID_PLATFORM=android-$api"
    -DANDROID_STL=c++_shared -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON
    "-DCMAKE_BUILD_TYPE=$build_type"
    "-DCMAKE_INSTALL_PREFIX=$prefix" -DCMAKE_INSTALL_LIBDIR=lib
    "-DCMAKE_PREFIX_PATH=$prefix" "-DCMAKE_FIND_ROOT_PATH=$prefix"
    -DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384
    -DCMAKE_EXE_LINKER_FLAGS=-Wl,-z,max-page-size=16384 -DBUILD_SHARED_LIBS=ON)

run_phase() {
    local label=$1
    shift
    current_phase=$label
    printf '%s\n' "$label"
    "$@"
}

build_dependencies() {
    local vcpkg=${VCPKG_ROOT:?Set VCPKG_ROOT to the vcpkg release checkout}
    run_phase bootstrap-vcpkg "$vcpkg/bootstrap-vcpkg.sh" -disableMetrics
    export VCPKG_MAX_CONCURRENCY="$jobs"
    run_phase install-dependencies "$vcpkg/vcpkg" install \
        --x-manifest-root="$root/.github/vcpkg-android" \
        --overlay-triplets="$root/.github/vcpkg-android/triplets" \
        --triplet="$triplet" --x-install-root="$work/dependencies" \
        --binarysource=clear --clean-after-build
}

build_and_install() {
    run_phase configure-fluidsynth cmake -S "$root" -B "$work/build/fluidsynth" "${args[@]}" \
        -Denable-opensles=ON -Denable-floats=ON -Denable-oboe=ON \
        -Denable-dbus=OFF -Denable-oss=OFF -Denable-openmp=OFF \
        -DCMAKE_CROSSCOMPILING_EMULATOR=android-test-launcher.sh
    run_phase build-fluidsynth cmake --build "$work/build/fluidsynth" --parallel "$jobs"
    run_phase build-tests cmake --build "$work/build/fluidsynth" --target check --parallel "$jobs"
    run_phase install-fluidsynth cmake --install "$work/build/fluidsynth"
    run_phase configure-assetloader cmake -S "$root/doc/android/fluidsynth-assetloader" \
        -B "$work/build/assetloader" "${args[@]}" \
        "-DCMAKE_C_FLAGS=-I$prefix/include" \
        "-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384 -L$prefix/lib"
    run_phase build-assetloader cmake --build "$work/build/assetloader" --parallel "$jobs"
}

package_libraries() {
    current_phase=package-libraries
    local toolchains=("$ndk"/toolchains/llvm/prebuilt/*)
    local toolchain=${toolchains[0]}
    if [[ ! -d "$toolchain" ]]; then
        printf 'Android NDK LLVM prebuilt toolchain not found under %s\n' "$ndk" >&2
        return 1
    fi
    for feature in OBOE_SUPPORT OPENSLES_SUPPORT LIBSNDFILE_SUPPORT LIBSNDFILE_HASVORBIS; do
        grep -qx "#define $feature 1" "$work/build/fluidsynth/config.h"
    done
    local package="$work/package" library source target
    rm -rf "$package" "$work/tests"
    mkdir -p "$package/lib/$abi" "$package/include"

    for library in FLAC fluidsynth-assetloader fluidsynth oboe ogg opus sndfile vorbis vorbisenc vorbisfile c++_shared; do
        current_phase="package-lib$library.so"
        case "$library" in
            fluidsynth-assetloader) source="$work/build/assetloader/libfluidsynth-assetloader.so" ;;
            c++_shared) source="$toolchain/sysroot/usr/lib/$triple/libc++_shared.so" ;;
            *) source="$prefix/lib/lib$library.so" ;;
        esac
        target="$package/lib/$abi/lib$library.so"
        cp -L "$source" "$target"
        # NDK 32-bit libc++ uses 4 KB pages; 16 KB Android devices use 64-bit ABIs.
        if [[ "$library" == c++_shared && "$abi" != arm64-v8a && "$abi" != x86_64 ]]; then
            continue
        fi
        OBJDUMP="$toolchain/bin/llvm-objdump" bash "$root/contrib/check_elf_alignment.sh" "$target"
    done
    current_phase=package-headers
    cp -a "$prefix/include/fluidsynth.h" "$prefix/include/fluidsynth" "$package/include/"
    local tests="$work/build/fluidsynth/test" executable
    mkdir -p "$work/tests/lib"
    for executable in "$tests"/test_*; do
        [[ -f "$executable" && -x "$executable" ]] || continue
        cp -a "$executable" "$work/tests/"
    done
    cp -a "$tests/sf2" "$tests/test" "$tests/CTestTestfile.cmake" "$work/tests/"
    cp -a "$package/lib/$abi/." "$work/tests/lib/"
}

case "$phase" in
    build-dependencies) build_dependencies ;;
    build-and-install) build_and_install ;;
    package) package_libraries ;;
esac
