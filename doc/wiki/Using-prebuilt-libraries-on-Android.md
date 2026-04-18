# ⚙️ Using prebuilt libraries on Android

When linking against FluidSynth on Android builds, you have to link not only fluidsynth.so but also all the other libraries that are included in the release, and some that are not included but are part of the Android NDK such as OpenMP. If you fail to do so your project may compile, but will fail at runtime because of missing dependencies.

## CMake example

1. Add the fluidsynth library
```cmake
  add_library(libfluidsynth SHARED IMPORTED)
  set(SHARED_LIBRARY_SO ${CMAKE_CURRENT_SOURCE_DIR}/../fluidsynth/lib/${CMAKE_ANDROID_ARCH_ABI}/libfluidsynth.so)
  set_target_properties(libfluidsynth PROPERTIES IMPORTED_LOCATION ${SHARED_LIBRARY_SO})
```

2. Add the dependencies included in the release
```cmake
	add_library(libfluidsynth-assetloader SHARED IMPORTED)
	set_target_properties(libfluidsynth-assetloader PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../fluidsynth/lib/${CMAKE_ANDROID_ARCH_ABI}/libfluidsynth-assetloader.so)
```

Repeat the process with all the dependencies included (libFLAC, liboboe, libopus...).

3. Add the dependencies included in Android NDK (OpenMP)
```cmake
find_package(OpenMP REQUIRED)
```

4. Put the pieces together under a single library
```cmake
	add_library(fluidsynth INTERFACE)
	target_link_libraries(fluidsynth INTERFACE
		libFLAC
		libfluidsynth-assetloader
		libgio-2.0
		libglib-2.0
		libgmodule-2.0
		libgobject-2.0
		libgthread-2.0
		libinstpatch-1.0
		liboboe
		libogg
		libopus
		libpcre
		libpcreposix
		libsndfile
		libvorbis
		libvorbisenc
		libvorbisfile
		libfluidsynth
		OpenMP::OpenMP_CXX
	)
```

5. And finally link the library to your project!
```cmake
target_link_libraries(your_project PRIVATE fluidsynth)
```