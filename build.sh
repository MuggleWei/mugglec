#!/bin/bash

# handle argv
if [ "$#" -lt 1 ]; then
	echo "[ERROR] build.sh without build type"
	echo "[ERROR] Usage: build.sh <Debug|Release|RelWithDebInfo|Coverage>"
	exit 1
else
	# to lowercase
	BUILD_TYPE=$(echo $1 | tr '[:upper:]' '[:lower:]')
fi

origin_dir="$(dirname "$(readlink -f "$0")")"
user_local_dir=$HOME/.local
build_dir=$origin_dir/build
dep_search_path=$user_local_dir/usr
install_dir=$user_local_dir/usr

mkdir -p $build_dir
cd $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_PREFIX_PATH=$dep_search_path \
	-DBUILD_SHARED_LIBS=ON \
	-DMUGGLE_BUILD_STATIC_PIC=ON \
	-DMUGGLE_BUILD_TRACE=OFF \
	-DMUGGLE_BUILD_SANITIZER=OFF \
	-DBUILD_TESTING=OFF \
	-DMUGGLE_BUILD_EXAMPLE=OFF \
	-DMUGGLE_BUILD_BENCHMARK=OFF \
	-DMUGGLE_INSTALL_BIN=OFF

# make && make install
cmake --build .
cpack
