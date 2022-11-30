#!/bin/bash

# handle argv
if [ "$#" -lt 1 ]; then
	echo "[ERROR] build.sh without build type"
	echo "[ERROR] Usage: build.sh <Debug|Release>"
	exit 1
else
	# to lowercase
	BUILD_TYPE=$(echo $1 | tr '[:upper:]' '[:lower:]')
fi

origin_dir="$(dirname "$(readlink -f "$0")")"
user_local_dir=$HOME/.local
build_dir=$origin_dir/build
install_dir=$user_local_dir/usr

mkdir -p $build_dir
cd $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DMUGGLE_BUILD_SHARED_LIB=ON \
	-DMUGGLE_BUILD_STATIC_PIC=ON \
	-DMUGGLE_BUILD_EXAMPLE=OFF \
	-DMUGGLE_BUILD_TESTING=OFF \
	-DMUGGLE_BUILD_BENCHMARK=OFF \
	-DMUGGLE_BUILD_TRACE=OFF \
	-DMUGGLE_EXTRA_PREFIX_PATH=$install_dir \
	-DMUGGLE_INSTALL_BIN=OFF \
	-DCMAKE_INSTALL_PREFIX=$install_dir

# make && make install
cmake --build .
cpack
