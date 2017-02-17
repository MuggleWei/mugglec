cmake_minimum_required (VERSION 2.8.9)
project(muggleLib)

set(targetLinks "")
set(targetDependencies "")
set(is_pure_c ON)

# cmake
set(muggle_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/cmake/muggle_cmake_common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/add_project.cmake)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# base
if (WIN32)
	set(targetLinks "")
else()
	set(targetLinks pthread dl)
endif()
set(targetDependencies "")
set(is_pure_c ON)
addMuggleLib("${muggle_base}" "${is_pure_c}" "MUGGLE_BASE_EXPORTS" "base" "${targetLinks}" "${targetDependencies}")

# memory pool
set(targetLinks "")
set(targetDependencies "")
set(is_pure_c ON)
addMuggleLib("${muggle_memory_pool}" "${is_pure_c}" "MUGGLE_MEMORY_POOL_EXPORTS" "mem_pool" "${targetLinks}" "${targetDependencies}")

# base cpp
set(targetLinks ${muggle_base})
set(targetDependencies ${muggle_base})
set(is_pure_c OFF)
addMuggleLib("${muggle_base_cpp}" "${is_pure_c}" "MUGGLE_BASE_CPP_EXPORTS" "base_cpp" "${targetLinks}" "${targetDependencies}")

# options for build example and test
option(BUILD_EXAMPLE "Build example" ON)
option(BUILD_RUN_TEST "Build test" ON)

if (${BUILD_EXAMPLE})
	add_subdirectory(example)
endif()

if (${BUILD_RUN_TEST})
	# find googletest source
	include(${CMAKE_CURRENT_LIST_DIR}/cmake/FindGTestSrc.cmake)
	if(GTESTSRC_FOUND)
	
		if (WIN32)
			# without this, will cause error: Debug Assertion Failed! Expression: __acrt_first_block == header
			SET(BUILD_SHARED_LIBS ON CACHE BOOL "build google test as dynamic lib")
		endif()
		
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/gtest)
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
	endif()
endif()
