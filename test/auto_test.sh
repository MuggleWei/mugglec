#!/bin/bash
build_dir=../build
bin_dir=${build_dir}/bin
report_dir=${bin_dir}/UnitTest

unit_tests=(UnitTest_File UnitTest_Str UnitTest_Atomic)

for item in ${unit_tests[@]};do
	if [ -f "${bin_dir}/${item}" ]; then
    	${bin_dir}/${item} --gtest_output=xml:${report_dir}/${item}_report.xml
	fi
done