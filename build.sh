#!/bin/bash

# handle argv
if [ "$#" -lt 2 ]; then
	echo "[ERROR] build.sh without build type"
	echo "[ERROR] Usage: build.sh <Debug|Release|RelWithDebInfo|Coverage> <shared|static>"
	exit 1
fi

# build type
BUILD_TYPE=$(echo $1 | tr '[:upper:]' '[:lower:]')

# lib type
if [ "$#" -gt 1 ]; then
	LIB_TYPE=$(echo $2 | tr '[:upper:]' '[:lower:]')
else
	LIB_TYPE="shared"
fi

if [ "$LIB_TYPE" = "shared" ]; then
	CMAKE_SHARED_LIBS=ON
else
	CMAKE_SHARED_LIBS=OFF
fi

echo "build type: $BUILD_TYPE"
echo "lib type: $LIB_TYPE"
echo "CMAKE_SHARED_LIB: $CMAKE_SHARED_LIBS"

# variables
origin_dir="$(dirname "$(readlink -f "$0")")"
build_dir=$origin_dir/build
install_dir=$origin_dir/dist
user_local_dir=$HOME/.local
dep_search_path=$user_local_dir/usr

if [ -d $build_dir ]; then
	rm -rf $build_dir
fi
mkdir -p $build_dir

if [ -d $install_dir ]; then
	rm -rf $install_dir
fi

cd $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_PREFIX_PATH=$dep_search_path \
	-DBUILD_SHARED_LIBS=$CMAKE_SHARED_LIBS \
	-DMUGGLE_BUILD_STATIC_PIC=ON \
	-DMUGGLE_BUILD_TRACE=OFF \
	-DMUGGLE_BUILD_SANITIZER=OFF \
	-DBUILD_TESTING=OFF \
	-DMUGGLE_BUILD_EXAMPLE=OFF \
	-DMUGGLE_BUILD_BENCHMARK=OFF \
	-DMUGGLE_INSTALL_BIN=OFF \
	-DCMAKE_INSTALL_PREFIX=$install_dir

	#-DCMAKE_C_FLAGS="-march=native" \
	#-DCMAKE_CXX_FLAGS="-march=native" \

# make && make install
cmake --build .
cmake --build . --target install
