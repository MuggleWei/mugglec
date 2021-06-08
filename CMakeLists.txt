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
set(MUGGLE_BUILD_ONLY_C ON)
# option(MUGGLE_BUILD_ONLY_C "Only build muggle c lib" OFF) # deprecate all cpp file
option(MUGGLE_BUILD_TRACE "If build type is debug then build with trace info in source codes" OFF)
option(MUGGLE_BUILD_SHARED_LIB "build shared or static library" ON)
option(MUGGLE_BUILD_STATIC_PIC "build static library with position independent code flag" ON)
option(MUGGLE_BUILD_EXAMPLE "Build muggleCC example" OFF)
option(MUGGLE_BUILD_TESTING "Build muggleCC test" OFF)
option(MUGGLE_BUILD_BENCHMARK "Build muggleCC benchmark" OFF)
set(MUGGLE_EXTRA_PREFIX_PATH "" CACHE STRING "extra prefix path for cmake FIND_XXX")
option(BUILD_TESTING "Buid testing" OFF)
# option(MUGGLE_CRYPT_OPTIMIZATION "Enable crypt optimization(use source codes extract from openssl)" ON)
set(MUGGLE_CRYPT_OPTIMIZATION ON)
# option(MUGGLE_CRYPT_COMPARE_OPENSSL "Link openssl in unittest for compare result" OFF)
set(MUGGLE_CRYPT_COMPARE_OPENSSL ON)

# version
set(MUGGLE_CC_VER_MAJOR 0)
set(MUGGLE_CC_VER_MINOR 0)
set(MUGGLE_CC_VER_BUILD 0)

set(MUGGLE_CC_VERSION "${MUGGLE_CC_VER_MAJOR}.${MUGGLE_CC_VER_MINOR}.${MUGGLE_CC_VER_BUILD}")
set(MUGGLE_CC_SOVERSION "${MUGGLE_CC_VER_MAJOR}")

# output options
message("-- version ${MUGGLE_CC_VERSION}")
message("-- option MUGGLE_BUILD_TRACE ${MUGGLE_BUILD_TRACE}")
message("-- option MUGGLE_BUILD_SHARED_LIB ${MUGGLE_BUILD_SHARED_LIB}")
message("-- option MUGGLE_BUILD_STATIC_PIC ${MUGGLE_BUILD_STATIC_PIC}")
message("-- option MUGGLE_BUILD_EXAMPLE ${MUGGLE_BUILD_EXAMPLE}")
message("-- option MUGGLE_BUILD_TESTING ${MUGGLE_BUILD_TESTING}")
message("-- option MUGGLE_BUILD_BENCHMARK ${MUGGLE_BUILD_BENCHMARK}")
message("-- option MUGGLE_EXTRA_PREFIX_PATH ${MUGGLE_EXTRA_PREFIX_PATH}")
message("-- option BUILD_TESTING ${BUILD_TESTING}")
message("-- option MUGGLE_CRYPT_OPTIMIZATION ${MUGGLE_CRYPT_OPTIMIZATION}")
message("-- option MUGGLE_CRYPT_COMPARE_OPENSSL ${MUGGLE_CRYPT_COMPARE_OPENSSL}")

if ("${MUGGLE_EXTRA_PREFIX_PATH}" STREQUAL "")
else()
	list(APPEND CMAKE_PREFIX_PATH ${MUGGLE_EXTRA_PREFIX_PATH})
	message("-- cmake prefix path: ${CMAKE_PREFIX_PATH}")
endif()


if (MUGGLE_BUILD_SHARED_LIB)
	set(MUGGLE_LIB_TYPE SHARED)
else()
	set(MUGGLE_LIB_TYPE STATIC)
endif()

if (MUGGLE_BUILD_TESTING)
	set(BUILD_TESTING ON)
endif()

# find openssl
set(MUGGLE_TEST_LINK_OPENSSL OFF)
if (MUGGLE_CRYPT_COMPARE_OPENSSL)
	find_package(OpenSSL QUIET)
	if (OPENSSL_FOUND)
		message("-- Find openssl - for compare crypt result in unittest")
		message("-- Openssl include dir: ${OPENSSL_INCLUDE_DIR}")
		message("-- Openssl libraries: ${OPENSSL_LIBRARIES}")
		set(MUGGLE_TEST_LINK_OPENSSL ON)
	else()
		message("-- Can't find Openssl, option MUGGLE_CRYPT_COMPARE_OPENSSL OFF")
	endif()
endif()

# include directories
include_directories(
	${CMAKE_CURRENT_LIST_DIR}
)

if (${MUGGLE_BUILD_ONLY_C})
	# muggle_add_project(muggleC ${CMAKE_CURRENT_LIST_DIR}/muggle/c ${MUGGLE_LIB_TYPE})
	# set_target_properties(muggleC PROPERTIES 
	# 	LINKER_LANGUAGE C
	# )
	# set(muggle_cc muggleC) 
	# target_compile_definitions(${muggle_cc}	PUBLIC MUGGLE_BUILD_C_ONLY)

	set(muggle_cc mugglecc)
	muggle_add_project(${muggle_cc} ${CMAKE_CURRENT_LIST_DIR}/muggle ${MUGGLE_LIB_TYPE})
	set_target_properties(${muggle_cc} PROPERTIES 
		LINKER_LANGUAGE C
		VERSION ${MUGGLE_CC_VERSION}
		SOVERSION ${MUGGLE_CC_SOVERSION}
	)
	if ((NOT ${MUGGLE_BUILD_SHARED_LIB}) AND (${MUGGLE_BUILD_STATIC_PIC}))
		set_target_properties(${muggle_cc} PROPERTIES
			POSITION_INDEPENDENT_CODE ON
		)
	endif()
	target_compile_definitions(${muggle_cc}	PUBLIC MUGGLE_BUILD_C_ONLY)
	target_compile_definitions(${muggle_cc}	PUBLIC
		MUGGLE_CC_VERSION_MAJOR=${MUGGLE_CC_VER_MAJOR}
		MUGGLE_CC_VERSION_MINOR=${MUGGLE_CC_VER_MINOR}
		MUGGLE_CC_VERSION_BUILD=${MUGGLE_CC_VER_BUILD}
	)
else()
	# deprecated all cpp sources
	# muggle_add_project(muggleCC ${CMAKE_CURRENT_LIST_DIR}/muggle ${MUGGLE_LIB_TYPE})
	# set(muggle_cc muggleCC)
	# target_compile_definitions(${muggle_cc} PUBLIC MUGGLE_BUILD_CC)
endif()

if (${MUGGLE_BUILD_TRACE})
	target_compile_definitions(${muggle_cc} PUBLIC MUGGLE_BUILD_TRACE)
endif()

# muggle benchmark
muggle_add_project(muggle_benchmark ${CMAKE_CURRENT_LIST_DIR}/muggle_benchmark ${MUGGLE_LIB_TYPE})
set_target_properties(muggle_benchmark PROPERTIES 
	VERSION ${MUGGLE_CC_VERSION}
	SOVERSION ${MUGGLE_CC_SOVERSION}
)
if ((NOT ${MUGGLE_BUILD_SHARED_LIB}) AND (${MUGGLE_BUILD_STATIC_PIC}))
	set_target_properties(muggle_benchmark PROPERTIES
		POSITION_INDEPENDENT_CODE ON
	)
endif()
target_link_libraries(muggle_benchmark ${muggle_cc})
add_dependencies(muggle_benchmark ${muggle_cc})

# set debug postfix
set_target_properties(${muggle_cc} muggle_benchmark
	PROPERTIES 
	DEBUG_POSTFIX d
)

# link thread and dll lib
find_package(Threads)
if (WIN32)
	target_link_libraries(${muggle_cc}
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
		synchronization
	)
	if (${MUGGLE_BUILD_TRACE})
		target_link_libraries(${muggle_cc} debug Dbghelp)
	endif()
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
	target_compile_definitions(muggle_benchmark
		PUBLIC MUGGLE_BENCHMARK_USE_DLL
		PRIVATE MUGGLE_BENCHMARK_EXPORTS 
	)
endif()

if (MUGGLE_CRYPT_OPTIMIZATION)
	target_compile_definitions(${muggle_cc}
		PUBLIC MUGGLE_CRYPT_OPTIMIZATION
	)
endif()


# install
install(TARGETS ${muggle_cc}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static
)
muggle_install_headers(${CMAKE_CURRENT_LIST_DIR}/muggle include/muggle)

# muggle test utils
if (MUGGLE_BUILD_TESTING)
	set(test_utils muggle_test_utils)
	muggle_add_project(${test_utils} ${CMAKE_CURRENT_LIST_DIR}/test_utils SHARED)
	add_dependencies(${test_utils} ${muggle_cc})
endif()

# functions
function(add_example name folder)
	message("add example ${name} ${folder}")
	
	set(name example_${name})

	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB tmp_cc ${folder}/*.cc)

	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "example"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
	endif(WIN32)
	add_dependencies(${name} ${muggle_cc} muggle_benchmark)
	target_link_libraries(${name}
		${muggle_cc}
		muggle_benchmark
	)
endfunction()

function(add_benchmark name folder)
	message("add benchmark ${name} ${folder}")
	
	set(name benchmark_${name})

	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB tmp_cc ${folder}/*.cc)

	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "benchmark"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
	endif(WIN32)
	add_dependencies(${name} muggle_benchmark)
	target_link_libraries(${name}
		muggle_benchmark
	)
endfunction()

function(add_gtest name folder)
	message("add test ${name} ${folder}")
	
	set(name test_${name})

	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB tmp_cc ${folder}/*.cc)

	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "test"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
	endif(WIN32)

	add_dependencies(${name} ${muggle_cc} ${test_utils})
	if (FOUND_GTEST_FROM_DOWNLOAD)
		add_dependencies(${name} ${GTEST_BOTH_LIBRARIES})
	endif()
	target_include_directories(${name} PUBLIC
		${GTEST_INCLUDE_DIRS}
	)
	target_link_libraries(${name}
		${muggle_cc}
		${test_utils}
		${GTEST_BOTH_LIBRARIES}
	)

	# link openssl
	if (MUGGLE_TEST_LINK_OPENSSL)
		if (${name} MATCHES "^test_crypt*")
			message("${name} link openssl")
			target_link_libraries(${name}
				${OPENSSL_LIBRARIES}
			)
			target_include_directories(${name} PUBLIC
				${OPENSSL_INCLUDE_DIR}
			)
			target_compile_definitions(${name}
				PUBLIC MUGGLE_TEST_LINK_OPENSSL
			)
		endif()
	endif()

	add_test(NAME ${name} COMMAND ${name})
endfunction()

# test
if (BUILD_TESTING)
	enable_testing()
	
	# search gtest first
	find_package(GTest)
	
	if (GTEST_FOUND)
		set(FOUND_GTEST_FROM_SEARCH ON)

		message("-- Find GTest - use gtest from search")
		message("-- GTest include directories: ${GTEST_INCLUDE_DIRS}")
		message("-- GTest both libraries: ${GTEST_BOTH_LIBRARIES}")
	else()
		# Download and unpack googletest at configure time
		configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/CMakeLists.txt.in googletest-download/CMakeLists.txt)
		execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
		if(result)
			message(FATAL_ERROR "CMake step for googletest failed: ${result}")
		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} --build .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
		if(result)
			message(FATAL_ERROR "Build step for googletest failed: ${result}")
		endif()

		# Prevent overriding the parent project's compiler/linker
		# settings on Windows
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

		# Add googletest directly to our build. This defines
		# the gtest and gtest_main targets.
		add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
						${CMAKE_CURRENT_BINARY_DIR}/googletest-build
						EXCLUDE_FROM_ALL)

		# The gtest/gtest_main targets carry header search path
		# dependencies automatically when using CMake 2.8.11 or
		# later. Otherwise we have to add them here ourselves.
		if (CMAKE_VERSION VERSION_LESS 2.8.11)
			include_directories("${gtest_SOURCE_DIR}/include")
		endif()

		# # Now simply link against gtest or gtest_main as needed. Eg
		# add_executable(example src/test/example/example.cpp)
		# target_link_libraries(example gtest_main)
		# add_test(NAME example_test COMMAND example)

		set(GTEST_INCLUDE_DIRS "${gtest_SOURCE_DIR}/include")
		set(GTEST_BOTH_LIBRARIES gtest_main)
		set(FOUND_GTEST_FROM_DOWNLOAD ON)

		message("-- Find GTest - use gtest from download")
	endif()
endif()

# example
if (${MUGGLE_BUILD_EXAMPLE})
	message("---------------------- example ----------------------")
	SUBDIRLIST(example_dirs ${CMAKE_CURRENT_LIST_DIR}/example)
	FOREACH(subdir ${example_dirs})
		add_example(${subdir} ${CMAKE_CURRENT_LIST_DIR}/example/${subdir})
	ENDFOREACH()
endif()

# test
if (MUGGLE_BUILD_TESTING)
	message("---------------------- test ----------------------")
	SUBDIRLIST(test_root_dir ${CMAKE_CURRENT_LIST_DIR}/test)
	FOREACH(subdir ${test_root_dir})
		add_gtest(${subdir} ${CMAKE_CURRENT_LIST_DIR}/test/${subdir})
	ENDFOREACH()
endif()

# benchmark
if (MUGGLE_BUILD_BENCHMARK)
	message("---------------------- benchmark ----------------------")
	SUBDIRLIST(benchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/benchmark)
	FOREACH(subdir ${benchmark_root_dir})
		add_benchmark(${subdir} ${CMAKE_CURRENT_LIST_DIR}/benchmark/${subdir})
	ENDFOREACH()
endif()
