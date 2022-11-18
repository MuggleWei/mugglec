#!/bin/bash

# get compiler
if [ $# -ne 1 ]; then
	echo "Usage: $0 [gcc|clang]"
	echo "Without specific compiler, use gcc"
	compiler=gcc
else
	compiler=$1
fi

if [[ "$compiler" == "gcc" ]]; then
	export CC=gcc
	export CXX=g++
elif [[ "$compiler" == "clang" ]]; then
	export CC=clang
	export CXX=clang++
else
	echo "Invalid compiler: $compiler"
	exit 1
fi

echo "use c compiler: $CC"
echo "use cxx compiler: $CXX"

# build
source_dir=$(dirname $(readlink -f "$0"))
cd $source_dir

rm -rf build
mkdir build
cd build

cmake .. \
	-DCMAKE_BUILD_TYPE=Coverage \
	-DMUGGLE_BUILD_SHARED_LIB=ON \
	-DMUGGLE_BUILD_STATIC_PIC=ON \
	-DMUGGLE_BUILD_EXAMPLE=OFF \
	-DMUGGLE_BUILD_TESTING=OFF \
	-DMUGGLE_BUILD_BENCHMARK=OFF \
	-DMUGGLE_BUILD_TRACE=OFF \
	-DMUGGLE_EXTRA_PREFIX_PATH=~/.local

# run test & coverage
make
make coverage
