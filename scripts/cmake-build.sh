#!/bin/bash

if [ ! -d build ]; then
	scripts/cmake-init.sh $1
fi

(cd build && ninja)
