cmake_minimum_required (VERSION 3.0.2)
project(muggleCC)

# set compile parameter
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
endif()

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# cmake
include(${CMAKE_CURRENT_LIST_DIR}/cmake/muggle_utils.cmake)

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# options
option(MUGGLE_BUILD_ONLY_C "Only build muggle c lib" OFF)
option(MUGGLE_BUILD_EXAMPLE "Build example" OFF)
option(MUGGLE_BUILD_TEST "Build test" OFF)
option(MUGGLE_BUILD_BENCHMARK "Build benchmark" OFF)
option(MUGGLE_BUILD_SHARED_LIB ON)

if (MUGGLE_BUILD_SHARED_LIB)
	set(MUGGLE_LIB_TYPE SHARED)
else()
	set(MUGGLE_LIB_TYPE STATIC)
endif()

# include directories
include_directories(
	${CMAKE_CURRENT_LIST_DIR}
)

if (${MUGGLE_BUILD_ONLY_C})
	muggle_add_project(muggleC ${CMAKE_CURRENT_LIST_DIR}/muggle/c ${MUGGLE_LIB_TYPE})
	set_target_properties(muggleC PROPERTIES 
		LINKER_LANGUAGE C
	)
	set(muggle_cc muggleC) 	
else()
	muggle_add_project(muggleCC ${CMAKE_CURRENT_LIST_DIR}/muggle ${MUGGLE_LIB_TYPE})
	
	set(muggle_cc muggleCC)
endif()

# link thread and dll lib
find_package(Threads)
if (WIN32)
target_link_libraries(${muggle_cc}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
	synchronization
)
else()
target_link_libraries(${muggle_cc}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
)
endif()

# export macro for windows
if (MUGGLE_BUILD_SHARED_LIB)
	target_compile_definitions(${muggle_cc}
		PUBLIC MUGGLE_CC_USE_DLL
		PRIVATE MUGGLE_CC_EXPORTS 
	)
endif()

# install
install(TARGETS ${muggle_cc}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static
)
muggle_install_headers(${CMAKE_CURRENT_LIST_DIR}/muggle include/muggle)

if (${MUGGLE_BUILD_EXAMPLE} AND NOT ${MUGGLE_BUILD_ONLY_C})
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/example)
endif()

if (${MUGGLE_BUILD_TEST} AND NOT ${MUGGLE_BUILD_ONLY_C})
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

if (${MUGGLE_BUILD_BENCHMARK} AND NOT ${MUGGLE_BUILD_ONLY_C})
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/benchmark)
endif()
