#!/bin/bash

if [ ! -d build ]; then
	source scripts/cmake-init.sh
fi

(cd build && ninja)
