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
origin_dir="$(dirname "$(readlink -f "$0")")"
user_local_dir=$HOME/.local
build_dir=$origin_dir/build
dep_search_path=$user_local_dir/usr

cd $origin_dir

rm -rf $build_dir
mkdir -p $build_dir
cd $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_PREFIX_PATH=$dep_search_path \
	-DBUILD_SHARED_LIBS=ON \
	-DMUGGLE_BUILD_STATIC_PIC=ON \
	-DMUGGLE_BUILD_TRACE=OFF \
	-DBUILD_TESTING=OFF \
	-DMUGGLE_BUILD_EXAMPLE=OFF \
	-DMUGGLE_BUILD_BENCHMARK=OFF \
	-DMUGGLE_BUILD_COVERAGE=ON \
	-DMUGGLE_INSTALL_BIN=OFF

# run test & coverage
make
make coverage
