#!/bin/bash

source ./build-env.sh

mkdir -p $DEV

pushd $DEV

        tar zxf $ARCHIVE_DIR/libiconv-${ICONV_VERSION}.tar.gz
        tar zxf $ARCHIVE_DIR/libffi-${FFI_VERSION}.tar.gz
        tar zxf $ARCHIVE_DIR/gettext-${GETTEXT_VERSION}.tar.gz
        tar xf $ARCHIVE_DIR/glib-${GLIB_VERSION}.${GLIB_EXTRAVERSION}.tar.xz
        tar zxf $ARCHIVE_DIR/oboe-${OBOE_VERSION}.tar.gz
        tar jxf $ARCHIVE_DIR/libsndfile-${SNDFILE_VERSION}.tar.bz2
        tar zxf $ARCHIVE_DIR/libinstpatch-${INSTPATCH_VERSION}.tar.gz
        tar zxf $ARCHIVE_DIR/libvorbis-${VORBIS_VERSION}.tar.gz
        tar zxf $ARCHIVE_DIR/libogg-${OGG_VERSION}.tar.gz
        tar xf $ARCHIVE_DIR/flac-${FLAC_VERSION}.tar.gz
        tar xf $ARCHIVE_DIR/opus-${OPUS_VERSION}.tar.gz

popd
