#!/usr/bin/env bash

set e+

mkdir -p cmake-build-debug
cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles" ../.
make
cd ..
