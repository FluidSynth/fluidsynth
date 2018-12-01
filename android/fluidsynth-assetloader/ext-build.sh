PWD=`pwd`

A_ABI=x86

#cp $PWD/../dist/$A_ABI/lib/pkgconfig/fluidsynth.pc $PWD/../dist/$A_ABI/lib/pkgconfig/fluidsynth-android.pc

mkdir -p build/$A_ABI && \
	cd build/$A_ABI && \
	LD_LIBRARY_PATH=$PWD/../dist/$A_ABI \
	PKG_CONFIG_PATH=../../../dist/$A_ABI/lib/pkgconfig/ \
	cmake \
		-DCMAKE_TOOLCHAIN_FILE=~/android-sdk-linux/ndk-bundle/build/cmake/android.toolchain.cmake \
		-DANDROID_PLATFORM=android-27 \
		-DANDROID_ABI=$A_ABI \
		../.. --trace-expand && \
	make

