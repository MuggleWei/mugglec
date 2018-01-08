cmake_minimum_required(VERSION 2.8.9)

# Define helper functions and macros used by Google Test.
include(${GTEST_SOURCE_DIR}/cmake/internal_utils.cmake) 
config_compiler_and_linker()

# include path
# include_directories(${CMAKE_CURRENT_LIST_DIR}/../thirdparty/gtest/googletest/include)
include_directories(${GTEST_INCLUDE_DIR})

# unit test
set(run_test_dependencies ${muggle_cc} gtest_main)
muggle_add_project(UnitTest_File "${CMAKE_CURRENT_LIST_DIR}/unittest/file" EXE)
target_link_libraries(UnitTest_File ${run_test_dependencies})

muggle_add_project(UnitTest_Str "${CMAKE_CURRENT_LIST_DIR}/unittest/str" EXE)
target_link_libraries(UnitTest_Str ${run_test_dependencies})

muggle_add_project(UnitTest_Atomic "${CMAKE_CURRENT_LIST_DIR}/unittest/atomic" EXE)
target_link_libraries(UnitTest_Atomic ${run_test_dependencies})

# folder
set_target_properties(
	UnitTest_File UnitTest_Str UnitTest_Atomic
	PROPERTIES
	FOLDER "UnitTest"
)