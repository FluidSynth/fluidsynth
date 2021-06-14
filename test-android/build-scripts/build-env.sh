#!/bin/bash

export ICONV_VERSION=1.16
# Use recent master libffi, because 3.3 is broken=checking host system type... Invalid configuration `arm-none-linux-eabi=machine `arm-none-linux not recognized
export FFI_VERSION=dd5bd03075149d7cf8441875c1a344e8beb57dde
export GETTEXT_VERSION=0.21
#need to switch to meson build system to use a more recent version 
export GLIB_VERSION=2.58
export GLIB_EXTRAVERSION=3
export OBOE_VERSION=1.5.0
export SNDFILE_VERSION=1.0.31
export INSTPATCH_VERSION=1.1.6
export VORBIS_VERSION=1.3.7
export OGG_VERSION=1.3.4
export OPUS_VERSION=1.3.1
# flac 1.3.3 is completely broken=pkgconfig is incorrectly installed, compilation failure, etc.; use recent master instead
export FLAC_VERSION=27c615706cedd252a206dd77e3910dfa395dcc49

export SCRIPTSDIR=$PWD
export DEV=$PWD/android-build-root/$ANDROID_ABI_CMAKE
export ARCHIVE_DIR=$PWD/archives
export DIST=$PWD/build-artifacts

# This is a symlink pointing to the real Android NDK
# Must be the same as $ANDROID_NDK_HOME see:
# https://github.com/actions/virtual-environments/blob/main/images/linux/Ubuntu2004-README.md
if [ -z "$NDK" ]; then
export NDK=~/Android/Sdk/ndk/21.3.6528147
fi

# All the built binaries, libs and their headers will be installed here
export PREFIX=$DEV/opt/android

# The path of standalone NDK toolchain
# Refer to https://developer.android.com/ndk/guides/standalone_toolchain.html
export NDK_TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64/

# Dont mix up .pc files from your host and build target
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
# setting PKG_CONFIG_PATH alone does not seem to be enough to avoid mixing up with the host, also set PKG_CONFIG_LIBDIR
export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH

# Set Android target API level
# when compiling with clang use at least 28 as this makes sure that android provides the posix_spawn functions, so the compilation of gettext will (should) work out of the box
# its probably a bug of gettext, if posix_spawn is not available it replaces it with its own implementation. Autotools of gettext set HAVE_POSIX_SPAWN==0 (which is correct) but for some reason REPLACE_POSIX_SPAWN==0 (which is wrong, as it should be 1).
# 
# NOTE=API 24 is required because it provides fseeko() and ftello() required by libflac
export ANDROID_API=24

# Tell configure what flags Android requires.
# Turn Wimplicit-function-declaration into errors. Else autotools will be fooled when checking for available functions (that in fact are NOT available) and compilation will fail later on.
# Also disable clangs integrated assembler, as the hand written assembly of libffi is not recognized by it, cf. https://crbug.com/801303
export CFLAGS="-fPIE -fPIC -I$PREFIX/include --sysroot=$NDK_TOOLCHAIN/sysroot -I$NDK_TOOLCHAIN/sysroot/usr/include -Werror=implicit-function-declaration"
export CXXFLAGS=$CFLAGS
export CPPFLAGS=$CXXFLAGS

export ARTIFACT_NAME=fluidsynth-android$ANDROID_API
