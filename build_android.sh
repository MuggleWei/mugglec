#!/bin/bash

# handle argv
if [ "$#" -lt 1 ]; then
	echo "[ERROR] build.sh without build type"
	echo "[ERROR] Usage: build.sh <Debug|Release|RelWithDebInfo>"
	exit 1
else
	# to lowercase
	BUILD_TYPE=$(echo $1 | tr '[:upper:]' '[:lower:]')
fi

origin_dir="$(dirname "$(readlink -f "$0")")"
build_dir=$origin_dir/build
install_dir=$origin_dir/dist

#abi=arm64-v8a
abi=armeabi-v7a

if [ -z "$ANDROID_NDK" ]; then
	echo "run without ANDROID_NDK"
	exit 1
else
	echo "ndk: $ANDROID_NDK"
fi

if [ -d $build_dir ]; then
	rm -rf $build_dir
fi
mkdir $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
	-DANDROID_ABI=$abi \
	-DCMAKE_INSTALL_PREFIX=$install_dir

# make && make install
cmake --build $build_dir
cmake --build $build_dir --target install
