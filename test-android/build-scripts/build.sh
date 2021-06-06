#!/bin/bash

        set -e

        source ./build-env.sh

# set environment variables

        # The cross-compile toolchain we use
        export ANDROID_TARGET=$ARCH-linux-android$ANDROID_TARGET_ABI
        #echo "##vso[task.setvariable variable=ANDROID_TARGET]$ANDROID_TARGET"
        export ANDROID_TARGET_API=$ANDROID_ARCH-linux-android$ANDROID_TARGET_ABI$ANDROID_API
        #echo "##vso[task.setvariable variable=ANDROID_TARGET_API]$ANDROID_TARGET_API"
        # Add the standalone toolchain to the search path.
        # FIXME: env. path should be at last; it depends on host glib tools to build some tests. 
        export PATH=$PATH:$PREFIX/bin:$PREFIX/lib:$PREFIX/include:$NDK_TOOLCHAIN/bin
        #echo "##vso[task.setvariable variable=PATH]$PATH"
        
        export LIBPATH0=$PREFIX/lib
        export LIBPATH1=$NDK_TOOLCHAIN/sysroot/usr/lib
        export LIBPATH2=$NDK_TOOLCHAIN/sysroot/usr/lib/$ARCH-linux-android$ANDROID_TARGET_ABI/$ANDROID_API
        export LIBPATH3=$NDK_TOOLCHAIN/sysroot/usr/lib/$ARCH-linux-android$ANDROID_TARGET_ABI
        export LDFLAGS="-pie -Wl,-rpath-link=$LIBPATH1 -L$LIBPATH1 -Wl,-rpath-link=$LIBPATH2 -L$LIBPATH2 -Wl,-rpath-link=$LIBPATH3 -L$LIBPATH3 -Wl,-rpath-link=$LIBPATH0 -L$LIBPATH0"
        #echo "##vso[task.setvariable variable=LDFLAGS]$LDFLAGS"
        # Tell configure what tools to use.
        export AR=$ANDROID_TARGET-ar
        #echo "##vso[task.setvariable variable=AR]$AR"
        export AS=$ANDROID_TARGET_API-clang
        #echo "##vso[task.setvariable variable=AS]$AS"
        export CC=$ANDROID_TARGET_API-clang
        #echo "##vso[task.setvariable variable=CC]$CC"
        export CXX=$ANDROID_TARGET_API-clang++
        #echo "##vso[task.setvariable variable=CXX]$CXX"
        export LD=ld.lld
        #echo "##vso[task.setvariable variable=LD]$LD"
        export STRIP=$ANDROID_TARGET-strip
        #echo "##vso[task.setvariable variable=STRIP]$STRIP"
        export RANLIB=$ANDROID_TARGET-ranlib
        #echo "##vso[task.setvariable variable=RANLIB]$RANLIB"


# libiconv

        echo "Building libiconv..."

        pushd $DEV/libiconv-$ICONV_VERSION
        ./configure \
          --host=$AUTOTOOLS_TARGET \
          --prefix=$PREFIX \
          --libdir=$LIBPATH0 \
          --disable-rpath \
          --enable-static \
          --disable-shared \
          --with-pic \
          --disable-maintainer-mode \
          --disable-silent-rules \
          --disable-gtk-doc \
          --disable-introspection \
          --disable-nls
        make -j$((`nproc`+1)) || exit 1
        make install || exit 1
        popd

# libffi

        echo "Building libffi..."

        pushd $DEV/libffi-$FFI_VERSION
        NOCONFIGURE=true autoreconf -v -i
        # install headers into the conventional ${PREFIX}/include rather than ${PREFIX}/lib/libffi-3.2.1/include.
        #sed -e '/^includesdir/ s/$(libdir).*$/$(includedir)/' -i include/Makefile.in
        #sed -e '/^includedir/ s/=.*$/=@includedir@/' -e 's/^Cflags: -I${includedir}/Cflags:/' -i libffi.pc.in
        LDFLAGS="$LDFLAGS -Wl,-soname,libffi.so" ./configure --host=$AUTOTOOLS_TARGET --prefix=$PREFIX --enable-static --disable-shared --libdir=$LIBPATH0
        make -j$((`nproc`+1)) || exit 1
        make install || exit 1
        popd

# gettext

        echo "Building gettext..."

        set -ex
        pushd $DEV/gettext-$GETTEXT_VERSION
        ./configure \
          --host=$AUTOTOOLS_TARGET \
          --prefix=$PREFIX \
          --libdir=$LIBPATH0 \
          --disable-rpath \
          --disable-libasprintf \
          --disable-java \
          --disable-native-java \
          --disable-openmp \
          --disable-curses \
          --enable-static \
          --disable-shared \
          --with-pic  \
          --disable-maintainer-mode \
          --disable-silent-rules \
          --disable-gtk-doc \
          --disable-introspection
        make -j$((`nproc`+1))
        make install
        popd

# glib

        echo "Building glib..."

        set -ex
        pushd $DEV/glib-$GLIB_VERSION.$GLIB_EXTRAVERSION
        cat << EOF > android.cache
glib_cv_long_long_format=ll
glib_cv_stack_grows=no
glib_cv_sane_realloc=yes
glib_cv_have_strlcpy=no
glib_cv_va_val_copy=yes
glib_cv_rtldglobal_broken=no
glib_cv_uscore=no
glib_cv_monotonic_clock=no
ac_cv_func_nonposix_getpwuid_r=no
ac_cv_func_posix_getpwuid_r=no
ac_cv_func_posix_getgrgid_r=no
glib_cv_use_pid_surrogate=yes
ac_cv_func_printf_unix98=no
ac_cv_func_vsnprintf_c99=yes
ac_cv_func_realloc_0_nonnull=yes
ac_cv_func_realloc_works=yes
EOF
        # Unfortunately, libffi is not linked against libgobject when compiling for aarch64, leading to the following error:
        #
        # /bin/bash ../libtool  --tag=CC   --mode=link aarch64-linux-android23-clang -Wall -Wstrict-prototypes -Wno-bad-function-cast -Werror=declaration-after-statement -Werror=missing-prototypes -Werror=implicit-function-declaration -Werror=pointer-arith -Werror=init-self -Werror=format=2 -Werror=missing-include-dirs -fPIE -fPIC -I/home/vsts/work/1/s/android-build-root/opt/android/include --sysroot=/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot -I/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot/include -Werror=implicit-function-declaration -fno-integrated-as -fno-strict-aliasing  -pie -Wl,-rpath-link=-I/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot/usr/lib -L/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot/usr/lib -L/home/vsts/work/1/s/android-build-root/opt/android/lib -L/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//lib -o gobject-query gobject-query.o ./libgobject-2.0.la ../glib/libglib-2.0.la -lintl -liconv  
        # libtool: link: aarch64-linux-android23-clang -Wall -Wstrict-prototypes -Wno-bad-function-cast -Werror=declaration-after-statement -Werror=missing-prototypes -Werror=implicit-function-declaration -Werror=pointer-arith -Werror=init-self -Werror=format=2 -Werror=missing-include-dirs -fPIE -fPIC -I/home/vsts/work/1/s/android-build-root/opt/android/include --sysroot=/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot -I/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot/include -Werror=implicit-function-declaration -fno-integrated-as -fno-strict-aliasing -pie -Wl,-rpath-link=-I/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot/usr/lib -o .libs/gobject-query gobject-query.o  -L/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//sysroot/usr/lib -L/home/vsts/work/1/s/android-build-root/opt/android/lib -L/usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//lib ./.libs/libgobject-2.0.so ../glib/.libs/libglib-2.0.so /home/vsts/work/1/s/android-build-root/opt/android/lib/libintl.so /home/vsts/work/1/s/android-build-root/opt/android/lib/libiconv.so -pthread -L/home/vsts/work/1/s/android-build-root/opt/android/lib
        # /usr/local/lib/android/sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64//bin/../lib/gcc/aarch64-linux-android/4.9.x/../../../../aarch64-linux-android/bin/ld: warning: libffi.so, needed by ./.libs/libgobject-2.0.so, not found (try using -rpath or -rpath-link)
        # ./.libs/libgobject-2.0.so: undefined reference to `ffi_type_sint32@LIBFFI_BASE_8.0'
        # ./.libs/libgobject-2.0.so: undefined reference to `ffi_prep_cif@LIBFFI_BASE_8.0'
        #
        # So, just add it to LDFLAGS to make sure it's always linked.
        # libz.so is also missing...
        #if [ "$ARCH" == "aarch64" ] ; then
        #FFILIB=`pkg-config --libs libffi`
        #echo $FFILIB
        #export LDFLAGS="$LDFLAGS $FFILIB -lz"
        #unset FFILIB ;
        #fi

        chmod a-x android.cache
        NOCONFIGURE=true ./autogen.sh
        ./configure \
          --host=$ANDROID_TARGET \
          --prefix=$PREFIX \
          --libdir=$LIBPATH0 \
          --disable-dependency-tracking \
          --cache-file=android.cache \
          --enable-included-printf \
          --with-pcre=no \
          --enable-libmount=no \
          --enable-xattr=no \
          --with-libiconv=gnu \
          --disable-static \
          --enable-shared \
          --with-pic \
          --disable-maintainer-mode \
          --disable-silent-rules
        make -j$((`nproc`+1)) || exit 1
        make install || exit 1
        popd


# ogg

        echo "Building libogg..."

        parameters_cmakeArgs="-DINSTALL_DOCS=0" parameters_sourceDir=$DEV/libogg-$OGG_VERSION parameters_workDir=  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh
        ls -la $DEV/libogg-$OGG_VERSION/build_$ANDROID_ABI_CMAKE/CMakeFiles/ || exit 1

# vorbis

        echo "Building libvorbis..."

        parameters_cmakeArgs=  parameters_sourceDir=$DEV/libvorbis-$VORBIS_VERSION parameters_workDir=  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh
        ls -la $DEV/libvorbis-$VORBIS_VERSION/build_$ANDROID_ABI_CMAKE/CMakeFiles/ || exit 1

# flac

        echo "Building libFLAC..."

        parameters_cmakeArgs="-DCMAKE_C_STANDARD=99 -DCMAKE_C_STANDARD_REQUIRED=1 -DWITH_ASM=0 -DBUILD_CXXLIBS=0 -DBUILD_PROGRAMS=0 -DBUILD_EXAMPLES=0 -DBUILD_DOCS=0 -DINSTALL_MANPAGES=0" parameters_sourceDir=$DEV/flac-$FLAC_VERSION parameters_workDir=  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh
        ls -la $DEV/flac-$FLAC_VERSION/build_$ANDROID_ABI_CMAKE/CMakeFiles/ || exit 1

# opus

        echo "Building libopus..."

        parameters_cmakeArgs="-DBUILD_PROGRAMS=0 -DOPUS_MAY_HAVE_NEON=1 -DCMAKE_C_STANDARD=99 -DCMAKE_C_STANDARD_REQUIRED=1" parameters_sourceDir=$DEV/opus-$OPUS_VERSION parameters_workDir=  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh
        ls -la $DEV/opus-${OPUS_VERSION}/build_$ANDROID_ABI_CMAKE/CMakeFiles/ || exit 1


# sndfile

        echo "Building libsndfile..."

        parameters_cmakeArgs="-DBUILD_PROGRAMS=0 -DBUILD_EXAMPLES=0" parameters_sourceDir=$DEV/libsndfile-$SNDFILE_VERSION parameters_workDir=  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh
        ls -la $DEV/libsndfile-$SNDFILE_VERSION/build_$ANDROID_ABI_CMAKE/CMakeFiles/ || exit 1

# oboe

        echo "Building oboe..."

        parameters_cmakeArgs= parameters_sourceDir=$DEV/oboe-$OBOE_VERSION parameters_workDir=  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh || exit 1
        # parameters_installCommand didn't work, ending up to copy liboboe.so into $PREFIX/include. Replacing it with the direct commands here.
        cp $DEV/oboe-$OBOE_VERSION/build_$ANDROID_ABI_CMAKE/liboboe.so $PREFIX/lib
        cp -ur $DEV/oboe-$OBOE_VERSION/include/oboe $PREFIX/include
        
        set -ex
        # create a custom pkgconfig file for oboe to allow fluidsynth to find it
        cat << EOF > $PKG_CONFIG_PATH/oboe-1.0.pc
prefix=${PREFIX}
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include
Name: Oboe
Description: Oboe library
Version: ${OBOE_VERSION}
Libs: -L\${libdir} -loboe -landroid -llog
Cflags: -I\${includedir}
EOF
        cat $PKG_CONFIG_PATH/oboe-1.0.pc || exit 1

# instpatch

        echo "Building libinstpatch..."

        parameters_cmakeArgs=  parameters_sourceDir=$DEV/libinstpatch-$INSTPATCH_VERSION parameters_workDir=  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh || exit 1

# fluidsynth

        # build
        echo "Building fluidsynth..."

        # FIXME: On arm64 it fails to build fluidsynth executable due to a bunch of library resolution failures...
        # To avoid the entire build failures, we ignore the 
        # exit coode here and go on with fake executable file.
        # It is not runnable on Android anyways.
        parameters_cmakeArgs="-Denable-opensles=1 -Denable-floats=1 -Denable-oboe=1 -Denable-dbus=0 -Denable-oss=0" parameters_sourceDir=../.. parameters_workDir=  parameters_condition=  parameters_installCommand='echo success' bash ./build-call-cmake.sh || echo "Failed to build fluidsynth, but it is expected. We continue build..." && touch ../../build_$ANDROID_ABI_CMAKE/src/fluidsynth

        # TBD: test (there should be a complete Android project that installs apk, launches on android device, loads native tests there through JNI as a library, and run them, automatically.)

        # install
        set -ex
        pushd ../../build_$ANDROID_ABI_CMAKE
        make install
        popd

# fluidsynth-assetloader

        echo "Building fluidsynth-assetloader..."

        parameters_cmakeArgs=  parameters_sourceDir=../../../ parameters_workDir=doc/android/fluidsynth-assetloader  parameters_condition=  parameters_installCommand= bash ./build-call-cmake.sh

# dist

        mkdir -p $DIST/lib/$ANDROID_ABI_CMAKE
        echo "Entering $DIST/lib/$ANDROID_ABI_CMAKE ..."
        pushd $DIST/lib/$ANDROID_ABI_CMAKE
        cp -LR $PREFIX/lib/* .
        ls -Rg .
        rm -rf *.dll *.alias gettext/ libtextstyle.* *.a *.la
        rm -f *.so.*
        mkdir -p $DIST/include
        pushd $DIST/include
        cp -a $PREFIX/include/fluidsynth* .
        popd
        popd
        echo "dist $ANDROID_ABI_CMAKE done."
