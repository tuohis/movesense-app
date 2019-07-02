#!/bin/bash

if [ $# -lt 1 ]; then
	echo "Usage: $0 app-folder-name [Release | Debug]"
    echo "  - Build type defaults to Debug"
    exit 1
fi

if [ $# -gt 1 ]; then
	if [ $2 != "Release" ] && [ $2 != "Debug" ]; then
		echo "Build type must be one of [Release | Debug]"
		exit 2
	fi
	BUILD_TYPE=$2
else
	BUILD_TYPE="Debug"
fi

BUILD_DIR="build$BUILD_TYPE"
BUILD_PATH="$1/$BUILD_DIR"

if [ ! -d $BUILD_PATH ]; then
	echo "Build dir $BUILD_PATH doesn't exist; initializing.."
	scripts/cmake-init.sh $1 $BUILD_DIR $BUILD_TYPE
fi

(cd $BUILD_PATH && ninja)
