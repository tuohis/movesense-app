#!/bin/bash

# Inits the build environment using CMake 
mkdir -p build
(cd build && cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../movesense-device-lib/MovesenseCoreLib/toolchain/gcc-nrf52.cmake -DMOVESENSE_CORE_LIBRARY=../movesense-device-lib/MovesenseCoreLib ../ -DCMAKE_BUILD_TYPE=Debug ../)
