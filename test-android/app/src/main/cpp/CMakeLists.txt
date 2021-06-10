# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.

project("fluidsynth_tests")

set (TEST_SRC_DIR ../../../../../test)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

AUX_SOURCE_DIRECTORY(tests TEST_SRC_FILES)
FILE(GLOB LINKED_OBJ_FILES ../../../../../build_${CMAKE_ANDROID_ARCH_ABI}/src/CMakeFiles/libfluidsynth-OBJ.dir/**/*.o)

message("-----------------------TEST_SRC_FILES---------------------------")
message("${TEST_SRC_FILES}")
message("-----------------------LINKED_OBJ_FILES---------------------------")
message("${LINKED_OBJ_FILES}")

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${TEST_SRC_FILES}
             ${LINKED_OBJ_FILES}
        )

target_include_directories(native-lib
        PRIVATE
        ../../../../../build_${CMAKE_ANDROID_ARCH_ABI}
        ../../../../build-scripts/build-artifacts/include
        ../../../../build-scripts/android-build-root/${CMAKE_ANDROID_ARCH_ABI}/opt/android/include
        ../../../../build-scripts/android-build-root/${CMAKE_ANDROID_ARCH_ABI}/opt/android/include/glib-2.0
        ../../../../build-scripts/android-build-root/${CMAKE_ANDROID_ARCH_ABI}/opt/android/lib/glib-2.0/include
        ../../../../../src
        ../../../../../src/bindings
        ../../../../../src/drivers
        ../../../../../src/midi
        ../../../../../src/rvoice
        ../../../../../src/sfloader
        ../../../../../src/synth
        ../../../../../src/utils
        ../../../../../test
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_directories(native-lib
        PRIVATE
        ../jniLibs/${CMAKE_ANDROID_ARCH_ABI})

target_link_libraries(native-lib
        ${log-lib}
        c++
        gobject-2.0
        glib-2.0
        sndfile
        instpatch-1.0
        OpenSLES
        oboe
        )
