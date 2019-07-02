#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 app-folder-name build-subfolder-name [Release | Debug]"
    echo "  - Build type defaults to Debug"
    exit 1
fi

if [ $# -gt 2 ]; then
    BUILD_TYPE=$3
else
    BUILD_TYPE="Debug"
fi

# Inits the build environment using CMake
BUILD_PATH="$1/$2"

echo "Initializing the build environment to $BUILD_PATH; build type is $BUILD_TYPE."

mkdir -p $BUILD_PATH
(cd $BUILD_PATH && cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../../movesense-device-lib/MovesenseCoreLib/toolchain/gcc-nrf52.cmake -DMOVESENSE_CORE_LIBRARY=../../movesense-device-lib/MovesenseCoreLib -DCMAKE_BUILD_TYPE=$BUILD_TYPE "../")
