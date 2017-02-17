cmake_minimum_required(VERSION 2.8.9)

# Define helper functions and macros used by Google Test.
include(${GTEST_SOURCE_DIR}/cmake/internal_utils.cmake) 
config_compiler_and_linker()

# include path
# include_directories(${CMAKE_CURRENT_LIST_DIR}/../thirdparty/gtest/googletest/include)
include_directories(${GTEST_INCLUDE_DIR})

# unit test
set(run_test_dependencies ${muggle_base_cpp} gtest_main)
addProject("UnitTest_File" "${CMAKE_CURRENT_LIST_DIR}/unittest/file" "${run_test_dependencies}" "EXE")
addProject("UnitTest_Str" "${CMAKE_CURRENT_LIST_DIR}/unittest/str" "${run_test_dependencies}" "EXE")

# folder
set_target_properties(
	UnitTest_File UnitTest_Str
	PROPERTIES
	FOLDER "UnitTest"
)