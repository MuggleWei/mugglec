#!/bin/bash

# handle argv
if [ "$#" -lt 1 ]; then
	echo "[ERROR] Usage: $0 <OS> <OS_VER>"
	echo "    @param OS: alpine | ubuntu | freebsd"
	echo "    @param OS_VER: os version"
	echo "e.g."
	echo "    $0 alpine 3.17"
	echo "    $0 ubuntu 22.04"
	exit 1
else
	OS=$1
	OS_VER=$2
fi

# cd script directory
origin_dir=$(readlink -f "$(dirname "$0")")
cd $origin_dir

mugglec_ver=$(head -n 1 version.txt)
REGISTRY=registry.mugglewei.com
echo "build image: $REGISTRY/mugglec:${mugglec_ver}-${OS}${OS_VER}"

# dev
docker build \
	--build-arg OS_VER=$OS_VER \
	-f dockerfiles/dev_${OS}.Dockerfile \
	-t ${REGISTRY}/${OS}-dev:${OS_VER} \
	.

# mugglec
docker build \
	--build-arg OS=${OS} \
	--build-arg OS_VER=$OS_VER \
	--build-arg REGISTRY=$REGISTRY \
	-f dockerfiles/mugglec.Dockerfile \
	-t ${REGISTRY}/mugglec:${mugglec_ver}-${OS}${OS_VER} \
	.
