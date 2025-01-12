#!/bin/bash

source ./build-env.sh

        mkdir -p archives
       	wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-${ICONV_VERSION}.tar.gz
        wget -O libffi-${FFI_VERSION}.tar.gz https://github.com/libffi/libffi/archive/${FFI_VERSION}.tar.gz
        wget http://ftp.gnu.org/pub/gnu/gettext/gettext-${GETTEXT_VERSION}.tar.gz
        wget http://ftp.gnome.org/pub/gnome/sources/glib/${GLIB_VERSION}/glib-${GLIB_VERSION}.${GLIB_EXTRAVERSION}.tar.xz
        wget -O oboe-${OBOE_VERSION}.tar.gz https://github.com/google/oboe/archive/${OBOE_VERSION}.tar.gz
        wget https://github.com/libsndfile/libsndfile/releases/download/${SNDFILE_VERSION}/libsndfile-${SNDFILE_VERSION}.tar.bz2
        wget -O libinstpatch-${INSTPATCH_VERSION}.tar.gz https://github.com/swami/libinstpatch/archive/refs/tags/v${INSTPATCH_VERSION}.tar.gz
        wget https://github.com/xiph/vorbis/releases/download/v${VORBIS_VERSION}/libvorbis-${VORBIS_VERSION}.tar.gz
        wget https://github.com/xiph/ogg/releases/download/v${OGG_VERSION}/libogg-${OGG_VERSION}.tar.gz
        wget -O flac-${FLAC_VERSION}.tar.gz https://github.com/xiph/flac/archive/${FLAC_VERSION}.tar.gz
        wget -O opus-${OPUS_VERSION}.tar.gz https://github.com/xiph/opus/archive/refs/tags/v${OPUS_VERSION}.tar.gz
        mv *.tar.gz *.tar.xz *.tar.bz2 $ARCHIVE_DIR
