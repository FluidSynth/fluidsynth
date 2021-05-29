#!/bin/bash

source ./build-env.sh

if [ -z $parameters_installCommand ] ; then
parameters_installCommand="make install"
fi

if [ -z $parameters_workDir ] ; then
parameters_workDir=$DEV
fi

source ./build-env.sh

    #set -ex
    pushd $parameters_sourceDir
    mkdir -p build_$ANDROID_ABI_CMAKE
    pushd build_$ANDROID_ABI_CMAKE
    
    # Invoke cmake in the most correctest way I've could find while try and erroring:
    #
    # The biggest pain point is that CMake does not seem to respect our existing cross compilation CFLAGS and LDFLAGS.
    # Hence we are passing them manually, once via Android flags and once for "Required" flags. The latter is necessary
    # to let cmake correctly probe for any existing header, function, library, etc.
    # Watch out: Sometimes the flags are passed as ;-limited list!


    cmake -G "Unix Makefiles" \
        -DCMAKE_MAKE_PROGRAM=make \
        -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DANDROID_NATIVE_API_LEVEL=$ANDROID_API \
        -DANDROID_ABI=$ANDROID_ABI_CMAKE \
        -DANDROID_TOOLCHAIN=$CC \
        -DANDROID_NDK=$NDK \
        -DANDROID_COMPILER_FLAGS="${CFLAGS}" \
        -DANDROID_LINKER_FLAGS=${LDFLAGS} \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_REQUIRED_FLAGS="${CFLAGS}" \
        -DCMAKE_REQUIRED_LINK_OPTIONS=${LDFLAGS} \
        -DCMAKE_INSTALL_PREFIX=$PREFIX \
        -DCMAKE_STAGING_PREFIX=$PREFIX \
        -DBUILD_SHARED_LIBS=1 \
        -DLIB_SUFFIX="" \
        $parameters_cmakeArgs ..
        #-DCMAKE_VERBOSE_MAKEFILE=1 \
    make -j$((`nproc`+1))
    $parameters_installCommand
    popd
    popd

