#!/bin/bash

origin_dir="$(dirname "$(readlink -f "$0")")"
build_dir=$origin_dir/build

cd $build_dir
xargs rm -v < install_manifest.txt
