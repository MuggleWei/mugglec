cmake_minimum_required (VERSION 2.8.9)
project(muggleLib)

set(targetLinks "")
set(targetDependencies "")
set(is_pure_c ON)

# cmake
set(muggle_source_dir ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/cmake/muggle_cmake_common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/add_project.cmake)

option(BUILD_MUGGLE_BASE_SEPARATE "Build muggle lib separately" ON)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if (${BUILD_MUGGLE_BASE_SEPARATE})
	# base c
	if (WIN32)
		set(targetLinks "")
	else()
		set(targetLinks pthread dl)
	endif()
	set(targetDependencies "")
	set(is_pure_c ON)
	addMuggleLib("${muggle_base_c}" "${is_pure_c}" "MUGGLE_BASE_C_EXPORTS" "base_c" "${targetLinks}" "${targetDependencies}")

	# utils c
	addMuggleLibRecurs(${muggle_utils_c} ON "MUGGLE_UTILS_C_EXPORTS" "utils_c" ${muggle_base_c} ${muggle_base_c})

	# base cpp
	if (WIN32)
		set(targetLinks ${muggle_base_c})
	else()
		set(targetLinks pthread dl ${muggle_base_c})
	endif()
	set(targetDependencies ${muggle_base_c})
	set(is_pure_c OFF)
	addMuggleLib("${muggle_base_cpp}" "${is_pure_c}" "MUGGLE_BASE_CPP_EXPORTS" "base_cpp" "${targetLinks}" "${targetDependencies}")

	# set folder in vs
	set_target_properties(
		${muggle_base_c} ${muggle_base_cpp} ${muggle_utils_c}
		PROPERTIES
		FOLDER "muggle"
	)
else()
	if (WIN32)
		set(targetLinks "")
	else()
		set(targetLinks pthread dl)
	endif()
	set(targetDependencies "")
	set(is_pure_c ON)
	list(APPEND exports_macros "MUGGLE_BASE_C_EXPORTS")
	list(APPEND exports_macros "MUGGLE_UTILS_C_EXPORTS")
	list(APPEND exports_macros "MUGGLE_BASE_CPP_EXPORTS")
	addMuggleLibRecurs(${muggle_base} OFF "${exports_macros}" "" "" "")

	# set folder in vs
	set_target_properties(
		${muggle_base}
		PROPERTIES
		FOLDER "muggle"
	)
endif()

# options for build example and test
option(BUILD_MUGGLE_EXAMPLE "Build example" ON)
option(BUILD_MUGGLE_TEST "Build test" ON)

if (${BUILD_MUGGLE_EXAMPLE} AND ${BUILD_MUGGLE_BASE_SEPARATE})
	add_subdirectory(example)
endif()

if (${BUILD_MUGGLE_TEST} AND ${BUILD_MUGGLE_BASE_SEPARATE})
	# find googletest source
	include(${CMAKE_CURRENT_LIST_DIR}/cmake/FindGTestSrc.cmake)
	if(GTESTSRC_FOUND)
	
		if (WIN32)
			# without this, will cause error: Debug Assertion Failed! Expression: __acrt_first_block == header
			SET(BUILD_SHARED_LIBS ON CACHE BOOL "build google test as dynamic lib")
		endif()
		
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/gtest)
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)

		set_target_properties(
			gtest gtest_main gmock gmock_main
			PROPERTIES
			FOLDER "thirdparty"
		)
	endif()
endif()
