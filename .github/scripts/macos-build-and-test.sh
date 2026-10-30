#!/usr/bin/env bash
# Build and test an SDK, then install it in <work-directory>/package.
# Dependency search paths come from the caller's environment and CMake options.
# Compatible with the Bash 3.2 shipped by macOS.
set -eo pipefail

usage() {
    cat <<'USAGE'
Usage: macos-build-and-test.sh [--check-pkg-config] [--check-xcode] [--stage-tests] <work-directory> [CMake options...]

Build with Ninja, run unit tests, build demos, and exercise default and staged
installation. The SDK is left in <work-directory>/package for the caller to archive.

  --check-pkg-config  Compile and run test/cmake/main.c against the installed SDK.
  --check-xcode       Also build with the Xcode generator.
  --stage-tests       Include compiled tests and fixtures in the SDK for another runner.
USAGE
}

build_test_and_install_sdk() {
    local source_dir=$1 work_dir=$2 jobs=$3
    shift 3
    local build_dir="$work_dir/build" package_dir="$work_dir/package"

    # Retain compiler and platform checks when reusing this build directory.
    # The previous invocation staged an SDK under a custom install prefix.
    cmake -G Ninja -S "$source_dir" -B "$build_dir" -UCMAKE_INSTALL_PREFIX "$@"
    cmake --build "$build_dir" --parallel "$jobs"
    CTEST_PARALLEL_LEVEL="${CTEST_PARALLEL_LEVEL:-$jobs}" \
        cmake --build "$build_dir" --target check --parallel "$jobs"
    cmake --build "$build_dir" --target demo --parallel "$jobs"
    # Each configuration must validate its own installation without stale libraries.
    rm -rf "$work_dir/default-install" "$package_dir"
    DESTDIR="$work_dir/default-install" cmake --install "$build_dir"

    # Framework destinations are computed from the configured prefix. Reconfigure
    # and rebuild before installing at the SDK's final location.
    cmake -S "$source_dir" -B "$build_dir" "$@" "-DCMAKE_INSTALL_PREFIX=$package_dir"
    cmake --build "$build_dir" --parallel "$jobs"
    cmake --install "$build_dir"
}

check_pkg_config_consumer() (
    local source_dir=$1 work_dir=$2
    local package_dir="$work_dir/package" pkg_config_flags
    local consumer_flags=()

    export PKG_CONFIG_PATH="$package_dir/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
    pkg_config_flags=$(pkg-config --cflags --libs --static fluidsynth)
    read -r -a consumer_flags <<< "$pkg_config_flags"
    c++ "$source_dir/test/cmake/main.c" -o "$work_dir/pkgconfig-consumer" \
        "-Wl,-rpath,$package_dir/lib" "${consumer_flags[@]}"
    "$work_dir/pkgconfig-consumer"
)

build_with_xcode() {
    local source_dir=$1 work_dir=$2 jobs=$3
    shift 3
    cmake -S "$source_dir" -B "$work_dir/xcode" -G Xcode "$@" \
        -Denable-sdl3=OFF -Denable-readline=OFF -Denable-dbus=OFF
    cmake --build "$work_dir/xcode" --config Debug --parallel "$jobs"
}

stage_tests_for_transfer() {
    local source_dir=$1 work_dir=$2
    local test_dir="$work_dir/build/test" bundle_dir="$work_dir/package/tests"
    local cmake_command registrations
    local test_prefix="$test_dir/" soundfont_prefix="$source_dir/sf2/"
    local comparison_script="$source_dir/cmake_admin/RunOutputTest.cmake"

    if [[ ! -f "$test_dir/CTestTestfile.cmake" ]]; then
        printf 'No built CTest suite found in %s\n' "$test_dir" >&2
        return 1
    fi
    cmake_command=$(sed -n 's/^CMAKE_COMMAND:INTERNAL=//p' "$work_dir/build/CMakeCache.txt")
    if [[ -z "$cmake_command" ]]; then
        echo 'CMake executable path is missing from the build cache.' >&2
        return 1
    fi

    rm -rf "$bundle_dir"
    mkdir -p "$bundle_dir"
    tar --exclude=CMakeFiles --exclude=Testing --exclude=cmake_install.cmake \
        -cf - -C "$test_dir" . | tar -xf - -C "$bundle_dir"
    cp -p "$source_dir"/sf2/*.yml "$bundle_dir/sf2/"
    cp -p "$source_dir/cmake_admin/RunOutputTest.cmake" "$bundle_dir/"

    # CTest records absolute build, source and tool paths. Transfer its original
    # test registrations using bundle-relative paths and the receiving runner's cmake.
    registrations=$(cat "$test_dir/CTestTestfile.cmake")
    registrations=${registrations//"$test_prefix"/./}
    registrations=${registrations//"$soundfont_prefix"/./sf2/}
    registrations=${registrations//"$comparison_script"/./RunOutputTest.cmake}
    registrations=${registrations//"$cmake_command"/cmake}
    printf '%s\n' "$registrations" > "$bundle_dir/CTestTestfile.cmake"
}

main() {
    local check_xcode=false check_pkg_config=false stage_tests=false
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --help|-h) usage; return ;;
            --check-xcode) check_xcode=true; shift ;;
            --stage-tests) stage_tests=true; shift ;;
            --check-pkg-config) check_pkg_config=true; shift ;;
            *) break ;;
        esac
    done
    if [[ $# -eq 0 || $1 == -* ]]; then
        usage >&2
        return 2
    fi
    local work_dir=$1
    shift
    local script_dir source_dir jobs framework_binary
    script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
    source_dir=$(cd "$script_dir/../.." && pwd)
    mkdir -p "$work_dir"
    work_dir=$(cd "$work_dir" && pwd)
    jobs=${BUILD_JOBS:-$(sysctl -n hw.logicalcpu)}

    build_test_and_install_sdk "$source_dir" "$work_dir" "$jobs" "$@"
    if grep -q '^enable-framework:BOOL=ON$' "$work_dir/build/CMakeCache.txt"; then
        framework_binary="$work_dir/package/Library/Frameworks/FluidSynth.framework/FluidSynth"
        if [[ ! -f "$framework_binary" ]]; then
            printf 'Installed framework binary is missing: %s\n' "$framework_binary" >&2
            return 1
        fi
    fi
    if [[ $check_pkg_config == true ]]; then
        check_pkg_config_consumer "$source_dir" "$work_dir"
    fi
    if [[ $check_xcode == true ]]; then
        build_with_xcode "$source_dir" "$work_dir" "$jobs" "$@"
    fi
    if [[ $stage_tests == true ]]; then
        stage_tests_for_transfer "$source_dir" "$work_dir"
    fi
}

main "$@"
