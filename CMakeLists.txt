cmake_minimum_required (VERSION 2.8.9)
project(muggleLib)

set(targetLinks "")
set(targetDependencies "")
set(is_pure_c ON)

# cmake
set(muggle_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/muggle_cmake_common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_project.cmake)

# base
if (WIN32)
	set(targetLinks "")
else()
	set(targetLinks "dl")
endif()
set(targetDependencies "")
set(is_pure_c ON)
addMuggleLib("${muggle_base}" "${is_pure_c}" "MUGGLE_BASE_EXPORTS" "base" "${targetLinks}" "${targetDependencies}")

# memory pool
set(targetLinks "")
set(targetDependencies "")
set(is_pure_c ON)
addMuggleLib("${muggle_memory_pool}" "${is_pure_c}" "MUGGLE_MEMORY_POOL_EXPORTS" "mem_pool" "${targetLinks}" "${targetDependencies}")

# cpp utils
set(targetLinks ${muggle_base})
set(targetDependencies ${muggle_base})
set(is_pure_c OFF)
addMuggleLib("${muggle_cpp_utils}" "${is_pure_c}" "MUGGLE_CPP_UTLS_EXPORTS" "cpp_utils" "${targetLinks}" "${targetDependencies}")

# test
option(BUILD_RUN_TEST "Build run test" ON)
if (${BUILD_RUN_TEST})
	add_subdirectory(run_test)
endif()