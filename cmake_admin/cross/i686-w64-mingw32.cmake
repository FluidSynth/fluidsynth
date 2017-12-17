# Cross compile toolchain configuration based on:
# http://www.cmake.org/Wiki/CMake_Cross_Compiling

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# 32 bit mingw-w64
SET(TOOLCHAIN_PREFIX "i686-w64-mingw32")

# compilers to use for C and C++
FIND_PROGRAM(CMAKE_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc)
FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++)
FIND_PROGRAM(CMAKE_RC_COMPILER NAMES ${TOOLCHAIN_PREFIX}-windres)
FIND_PROGRAM(PKG_CONFIG_EXECUTABLE ${TOOLCHAIN_PREFIX}-pkg-config)

# path to the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX}/sys-root/mingw)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
