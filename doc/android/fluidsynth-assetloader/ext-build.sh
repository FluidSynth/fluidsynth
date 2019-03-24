PWD=`pwd`

ABIS="x86 x86_64 armeabi-v7a arm64-v8a"

HOST_OS=`uname | tr [:upper:] [:lower:]`

ANDROID_NDK_PATH=~/android-sdk-$HOST_OS/ndk-bundle
CMAKEFILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake

for A_ABI in $ABIS ; do
    mkdir -p build/$A_ABI && \
	    cd build/$A_ABI && \
	    cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKEFILE -DANDROID_PLATFORM=android-27 -DANDROID_ABI=$A_ABI ../.. && \
	    make &&
	    cd ../.. ;
done
