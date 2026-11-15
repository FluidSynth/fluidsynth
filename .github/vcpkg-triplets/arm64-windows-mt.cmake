set(VCPKG_TARGET_ARCHITECTURE arm64)
# Static CRT to match -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded; DLLs because the fluidsynth executable and library both link SDL3
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_BUILD_TYPE release)
