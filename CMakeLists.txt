cmake_minimum_required (VERSION 3.18.6)
project(mugglec)

################################
# general config
################################

# print compiler
message("-- use c compiler ${CMAKE_C_COMPILER}")
message("-- use c++ compiler ${CMAKE_CXX_COMPILER}")

# set compile parameter
if (NOT CMAKE_CROSSCOMPILING)
	if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	elseif (${CMAKE_C_COMPILER_ID} MATCHES Clang)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
		add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
		add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
	endif()

	# NOTE: by default, without -march=native
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
else()
	if (ANDROID)
		message("-- cmake run cross compiling, building for android")
	else()
		message(FATAL_ERROR "-- building for unsupport platform")
	endif()
endif()

# set standard and print features
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

message("-- c compiler support features: ")
foreach(feature ${CMAKE_C_COMPILE_FEATURES})
	message("support feature: ${feature}")
endforeach()

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################
# include cmake
################################

# cmake
include(${CMAKE_CURRENT_LIST_DIR}/cmake/muggle_utils.cmake)

################################
# options
################################

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug, RelWithDebInfo, Release, MinSizeRel)")
option(BUILD_SHARED_LIBS "Build shared or static library" ON)
option(BUILD_TESTING "Build testing" OFF)

# # close generator multi config, set to CMAKE_BUILD_TYPE
# if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
# 	if (NOT CMAKE_BUILD_TYPE)
# 		set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug, RelWithDebInfo, Release, MinSizeRel)" FORCE)
# 	endif()
# endif()
# get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
# if (is_multi_config)
#     set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
# endif()

option(MUGGLE_BUILD_STATIC_PIC "build static library with position independent code flag" ON)
option(MUGGLE_BUILD_EXAMPLE "Build mugglec example" OFF)
option(MUGGLE_BUILD_STRESS_TEST "Build mugglec stress test" OFF)
option(MUGGLE_BUILD_BENCHMARK_UTILS "Build mugglec benchmark utils" OFF)
option(MUGGLE_BUILD_BENCHMARK "Build mugglec benchmark" OFF)
option(MUGGLE_BUILD_TRACE "If build type is debug then build with trace info in source codes" OFF)
option(MUGGLE_BUILD_SANITIZER "Compile mugglec with sanitizer" OFF)
option(MUGGLE_INSTALL_BIN "Install example, unittest and benchmark binaries" OFF)
# option(MUGGLE_CRYPT_OPTIMIZATION "Enable crypt optimization(use source codes extract from openssl)" ON)
set(MUGGLE_CRYPT_OPTIMIZATION ON)
# option(MUGGLE_CRYPT_COMPARE_OPENSSL "Link openssl in unittest for compare result" OFF)
set(MUGGLE_CRYPT_COMPARE_OPENSSL ON)
set(MUGGLE_BUILD_WITH_COV OFF)
set(MUGGLE_TEST_LINK_OPENSSL OFF)

if (BUILD_SHARED_LIBS)
	set(MUGGLE_LIB_TYPE SHARED)
	set(MUGGLE_C_USE_DLL ON)
else()
	set(MUGGLE_LIB_TYPE STATIC)
	set(MUGGLE_C_USE_DLL OFF)
endif()

################################
# coverage
################################

if (${CMAKE_BUILD_TYPE} MATCHES "[Cc]overage")
	set(BUILD_TESTING ON)

	if (${CMAKE_C_COMPILER_ID} MATCHES Clang)
		message("-- CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}, set MUGGLE_BUILD_WITH_COV ON")
		set(MUGGLE_BUILD_WITH_COV ON)
		set(COVERAGE_COMPILER_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
	elseif(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
		message("-- CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE} and find gcov in ${GCOV_PATH}, set MUGGLE_BUILD_WITH_COV ON")
		set(MUGGLE_BUILD_WITH_COV ON)
		set(COVERAGE_COMPILER_FLAGS "--coverage")
	else()
		message("-- Use compiler ${CMAKE_C_COMPILER_ID}, set MUGGLE_BUILD_WITH_COV OFF")
		set(MUGGLE_BUILD_WITH_COV OFF)
	endif()
else()
	message("-- CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}, set MUGGLE_BUILD_WITH_COV OFF")
	set(MUGGLE_BUILD_WITH_COV OFF)
endif()

if (MUGGLE_BUILD_WITH_COV)
	message("-- muggle build with cov, add compiler flags: ${COVERAGE_COMPILER_FLAGS}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
endif()

if (MUGGLE_BUILD_WITH_COV)
	if (${CMAKE_C_COMPILER_ID} MATCHES Clang)
		find_program(LLVM_PROFDATA llvm-profdata)
		find_program(LLVM_COV_PATH llvm-cov)
		if (LLVM_PROFDATA AND LLVM_COV_PATH)
			set(COV_OUT_NAME mugglec_coverage)
			add_custom_target(
				coverage
				COMMAND echo "run converage"
				COMMAND rm -rf ${COV_OUT_NAME}
				COMMAND rm -rf mugglec-*profraw
				COMMAND rm -rf mugglec.profdata
				COMMAND LLVM_PROFILE_FILE=mugglec-%m.profraw make test
				COMMAND ${LLVM_PROFDATA} merge mugglec-*.profraw -o mugglec.profdata
				COMMAND ${LLVM_COV_PATH} report ./lib/libmugglec.* -instr-profile=mugglec.profdata
				COMMAND ${LLVM_COV_PATH} show ./lib/libmugglec.* -instr-profile=mugglec.profdata -output-dir=${COV_OUT_NAME} -format=html
				)
		else()
			message(SEND_ERROR "failed generate coverage report, llvm-profdata or llvm-cov not found")
		endif()
	elseif(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
		find_program(LCOV_PATH lcov)
		find_program(GENHTML_PATH genhtml)
		if (LCOV_PATH AND GENHTML_PATH)
			set(COV_OUT_NAME mugglec_coverage)
			set(COV_DIR ./CMakeFiles/mugglec.dir)
			add_custom_target(
				coverage
				COMMAND echo "run converage"
				COMMAND rm -rf ${COV_OUT_NAME}.info ${COV_OUT_NAME}
				COMMAND ${LCOV_PATH} --zerocounters -d ${COV_DIR}
				COMMAND rm -rf ${COV_OUT_NAME} ${COV_OUT_NAME}.info
				COMMAND make test
				COMMAND ${LCOV_PATH} -d ${COV_DIR} -c -o ${COV_OUT_NAME}.info
				COMMAND ${GENHTML_PATH} ${COV_OUT_NAME}.info -o ${COV_OUT_NAME}
				)
		else()
			message(SEND_ERROR "failed generate coverage report, lcov or genhtml not found")
		endif()
	endif()
endif()

################################
# version
################################

# NTOE: don't use READ, it will add newline
#file(READ "version.txt" mugglec_version)
file(STRINGS "version.txt" mugglec_version)

string(REPLACE "-" ";" mugglec_semver_ext ${mugglec_version})
list(GET mugglec_semver_ext 0 mugglec_semver)
string(REPLACE "." ";" mugglec_semver_list ${mugglec_semver})

list(GET mugglec_semver_list 0 MUGGLE_C_VER_MAJOR)
list(GET mugglec_semver_list 1 MUGGLE_C_VER_MINOR)
list(GET mugglec_semver_list 2 MUGGLE_C_VER_PATCH)

set(MUGGLE_C_VERSION "${mugglec_version}")
set(MUGGLE_C_SOVERSION "${MUGGLE_C_VER_MAJOR}")

################################
# output options
################################

string(TOUPPER ${CMAKE_BUILD_TYPE} upper_cmake_build_type)

message("--------------------------------")

message("# mugglec version ${MUGGLE_C_VERSION}")
message("# c compiler: ${CMAKE_C_COMPILER}")
message("# c compiler version: ${CMAKE_C_COMPILER_VERSION}")
message("# c compiler flags: ${CMAKE_C_FLAGS}")
message("# c compiler build flags: ${CMAKE_C_FLAGS_${upper_cmake_build_type}}")
message("# cxx compiler: ${CMAKE_CXX_COMPILER}")
message("# cxx compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
message("# cxx compiler flags: ${CMAKE_CXX_FLAGS}")
message("# cxx compiler build flags: ${CMAKE_CXX_FLAGS_${upper_cmake_build_type}}")
message("# CMake version: ${CMAKE_VERSION}")
message("# CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("# mugglec option BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message("# mugglec option MUGGLE_BUILD_STATIC_PIC: ${MUGGLE_BUILD_STATIC_PIC}")
message("# mugglec option MUGGLE_BUILD_TRACE: ${MUGGLE_BUILD_TRACE}")
message("# mugglec option MUGGLE_BUILD_WITH_COV: ${MUGGLE_BUILD_WITH_COV}")
message("# mugglec option MUGGLE_BUILD_SANITIZER: ${MUGGLE_BUILD_SANITIZER}")
message("# mugglec option BUILD_TESTING: ${BUILD_TESTING}")
message("# mugglec option MUGGLE_BUILD_EXAMPLE: ${MUGGLE_BUILD_EXAMPLE}")
message("# mugglec option MUGGLE_BUILD_STRESS_TEST: ${MUGGLE_BUILD_STRESS_TEST}")
message("# mugglec option MUGGLE_BUILD_BENCHMARK_UTILS: ${MUGGLE_BUILD_BENCHMARK_UTILS}")
message("# mugglec option MUGGLE_BUILD_BENCHMARK: ${MUGGLE_BUILD_BENCHMARK}")
message("# mugglec option MUGGLE_CRYPT_OPTIMIZATION: ${MUGGLE_CRYPT_OPTIMIZATION}")
message("# mugglec option MUGGLE_CRYPT_COMPARE_OPENSSL: ${MUGGLE_CRYPT_COMPARE_OPENSSL}")
message("# mugglec option MUGGLE_INSTALL_BIN: ${MUGGLE_INSTALL_BIN}")

message("--------------------------------")

################################
# configure
################################

# check support
include(CheckIncludeFile)
check_include_file(linux/futex.h MUGGLE_C_HAVE_LINUX_FUTEX)
check_include_file(sys/futex.h MUGGLE_C_HAVE_SYS_FUTEX)

include(CheckCSourceCompiles)

check_c_source_compiles("
	#include <stdalign.h>
	int main() {
		alignas(128) int data;
		return 0;
	}
" MUGGLE_C_HAVE_ALIGNAS)

check_c_source_compiles("
	#include <stdlib.h>
	int main() {
		void *p = aligned_alloc(128, 128 * 16);
		free(p);
		return 0;
	}
" MUGGLE_C_HAVE_ALIGNED_ALLOC)

# detech endianness
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)  # NOTE: big endian is 1, little endian is 0
if (IS_BIG_ENDIAN)
	set(MUGGLE_C_IS_BIG_ENDIAN 1)
else()
	set(MUGGLE_C_IS_BIG_ENDIAN 0)
endif()

if (CMAKE_HOST_UNIX)
	find_package(Backtrace)
	if (Backtrace_FOUND)
		#message("Found Backtrace")
		#message("Backtrace header: ${Backtrace_HEADER}")
		#message("Backtrace libs: ${Backtrace_LIBRARIES}")

		set(MUGGLE_C_HAVE_BACKTRACE 1)
		set(MUGGLE_C_BACKTRACE_HEADER ${Backtrace_HEADER})
	endif()
endif()

set(mugglec_generated_dir ${CMAKE_CURRENT_BINARY_DIR}/generated)
configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/muggle/c/mugglec_config.h.in"
	"${mugglec_generated_dir}/muggle/c/mugglec_config.h")

################################
# mugglec
################################

set(muggle_c mugglec)

muggle_add_project(${muggle_c} ${CMAKE_CURRENT_LIST_DIR}/muggle ${MUGGLE_LIB_TYPE})
target_include_directories(${muggle_c} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	$<BUILD_INTERFACE:${mugglec_generated_dir}>
	$<INSTALL_INTERFACE:include>)

target_compile_definitions(${muggle_c} PRIVATE MUGGLE_C_EXPORTS)
set_target_properties(${muggle_c} PROPERTIES 
	LINKER_LANGUAGE C
	VERSION ${MUGGLE_C_VERSION}
	SOVERSION ${MUGGLE_C_SOVERSION}
	DEBUG_POSTFIX d
)

if ((NOT ${BUILD_SHARED_LIBS}) AND (${MUGGLE_BUILD_STATIC_PIC}))
	set_target_properties(${muggle_c} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
	)
endif()

# extra libraries
if (NOT CMAKE_CROSSCOMPILING)
	find_package(Threads)
	if (Threads_FOUND)
		message("-- mugglec link threads: ${CMAKE_THREAD_LIBS_INIT}")
		target_link_libraries(${muggle_c} ${CMAKE_THREAD_LIBS_INIT})
	else()
		message(FATAL_ERROR "Failed found threads")
	endif()
else()
	if (ANDROID)
		message("-- ignore find Threads when building for android")
	endif()
endif()

message("-- mugglec link dl libs: ${CMAKE_DL_LIBS}")
target_link_libraries(${muggle_c} ${CMAKE_DL_LIBS})

if (MSVC OR MINGW)
	message("-- mugglec link sync: synchronization")
	target_link_libraries(${muggle_c} synchronization)
endif()

if (Backtrace_FOUND)
	target_include_directories(${muggle_c} PRIVATE ${Backtrace_INCLUDE_DIRS})

	message("-- mugglec link backtrace lib: ${Backtrace_LIBRARIES}")
	target_link_libraries(${muggle_c} ${Backtrace_LIBRARIES})
endif()

if (MUGGLE_BUILD_TRACE)
	if (MSVC OR MINGW)
		message("-- mugglec link debug lib: debug Dbghelp")
		target_link_libraries(${muggle_c} debug Dbghelp)
	endif()
endif()

if (MUGGLE_BUILD_SANITIZER)
	if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
		(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
		target_compile_options(${muggle_c} PUBLIC
			-fsanitize=undefined
			-fsanitize=address
			-fsanitize=leak)
		target_link_options(${muggle_c} PUBLIC
			-fsanitize=undefined
			-fsanitize=address
			-fsanitize=leak)
	endif()
endif()

if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
	(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
	target_compile_options(${muggle_c} PUBLIC -save-temps=obj)
endif()

# install
include(GNUInstallDirs)
install(TARGETS ${muggle_c}
	EXPORT mugglecTargets
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/muggle" "${mugglec_generated_dir}/muggle"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	FILES_MATCHING
	PATTERN "*.h")

# cmake package config
set(mugglec_export_dir "${CMAKE_INSTALL_LIBDIR}/cmake/mugglec")
install(EXPORT mugglecTargets
	FILE mugglecTargets.cmake
	DESTINATION ${mugglec_export_dir})

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_LIST_DIR}/cmake/mugglecConfig.cmake.in"
    "${mugglec_generated_dir}/mugglecConfig.cmake"
    INSTALL_DESTINATION ${mugglec_export_dir}
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(
    "${mugglec_generated_dir}/mugglecConfigVersion.cmake"
	VERSION ${MUGGLE_C_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES
	"${mugglec_generated_dir}/mugglecConfig.cmake"
	"${mugglec_generated_dir}/mugglecConfigVersion.cmake"
	DESTINATION ${mugglec_export_dir})

# pkgconfig
configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/cmake/mugglec.pc.in"
	"${mugglec_generated_dir}/mugglec.pc" @ONLY)
install(FILES
	"${mugglec_generated_dir}/mugglec.pc"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# cpack
#include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR ${MUGGLE_C_VER_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MUGGLE_C_VER_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MUGGLE_C_VER_PATCH})
set(CPACK_PACKAGE_CONTACT "Muggle Wei <mugglewei@gmail.com>")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/MuggleWei/mugglec")
include(CPack)

################################
# test & benchmark utils
################################

# test utils
if (${BUILD_TESTING})
	if (MUGGLE_CRYPT_COMPARE_OPENSSL)
		find_package(OpenSSL 3 QUIET)
		if (OPENSSL_FOUND)
			message("-- Find openssl - for compare crypt result in unittest")
			message("-- Openssl include dir: ${OPENSSL_INCLUDE_DIR}")
			message("-- Openssl libraries: ${OPENSSL_LIBRARIES}")
			set(MUGGLE_TEST_LINK_OPENSSL ON)
		else()
			message("-- Can't find Openssl, option MUGGLE_TEST_LINK_OPENSSL OFF")
		endif()
	endif()

	set(test_utils muggle_test_utils)
	muggle_add_project(${test_utils} ${CMAKE_CURRENT_LIST_DIR}/test_utils SHARED)
	add_dependencies(${test_utils} ${muggle_c})
	target_include_directories(${test_utils} PUBLIC
		${CMAKE_CURRENT_LIST_DIR}
		${mugglec_generated_dir})
	if (MUGGLE_INSTALL_BIN)
		install(TARGETS ${test_utils}
			RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
			ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
			LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
	endif()
endif()

# benchmark utils
if (${MUGGLE_BUILD_BENCHMARK} OR ${MUGGLE_BUILD_BENCHMARK_UTILS})
	muggle_add_project(muggle_benchmark ${CMAKE_CURRENT_LIST_DIR}/muggle_benchmark ${MUGGLE_LIB_TYPE})
	set_target_properties(muggle_benchmark PROPERTIES 
		VERSION ${MUGGLE_C_VERSION}
		SOVERSION ${MUGGLE_C_SOVERSION}
	)
	set_target_properties(muggle_benchmark
		PROPERTIES 
		DEBUG_POSTFIX d
	)
	if ((NOT ${BUILD_SHARED_LIBS}) AND ${MUGGLE_BUILD_STATIC_PIC})
		set_target_properties(muggle_benchmark PROPERTIES POSITION_INDEPENDENT_CODE ON)
	endif()
	target_link_libraries(muggle_benchmark ${muggle_c})
	add_dependencies(muggle_benchmark ${muggle_c})
	
	if (BUILD_SHARED_LIBS)
		target_compile_definitions(muggle_benchmark
			PUBLIC MUGGLE_BENCHMARK_USE_DLL
			PRIVATE MUGGLE_BENCHMARK_EXPORTS 
		)
	endif()

	if (MUGGLE_INSTALL_BIN)
		install(TARGETS muggle_benchmark
			RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
			ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
			LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
	endif()
endif()

################################
# functions
################################

# functions
function(add_example name folder)
	message("add example ${name} ${folder}")
	
	set(name example_${name})

	file(GLOB_RECURSE tmp_h ${folder}/*.h)
	file(GLOB_RECURSE tmp_c ${folder}/*.c)
	file(GLOB_RECURSE tmp_cpp ${folder}/*.cpp)
	file(GLOB_RECURSE tmp_cc ${folder}/*.cc)

	if (MSVC OR MINGW)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "example"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		if (APPLE)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "@executable_path/../lib"
			)
		else ()
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "\$ORIGIN/../lib"
			)
		endif()
	endif(MSVC OR MINGW)
	add_dependencies(${name} ${muggle_c})
	target_include_directories(${name} PUBLIC
		${folder}
	)
	target_link_libraries(${name} ${muggle_c})

	target_compile_definitions(${name} PRIVATE MUGGLE_HOLD_LOG_MACRO)

	if (MUGGLE_INSTALL_BIN)
		install(TARGETS ${name} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
	endif()
endfunction()

function(add_stress_test name folder)
	message("add stress_test ${name} ${folder}")

	set(name stress_test_${name})

	file(GLOB_RECURSE tmp_h ${folder}/*.h)
	file(GLOB_RECURSE tmp_c ${folder}/*.c)
	file(GLOB_RECURSE tmp_cpp ${folder}/*.cpp)
	file(GLOB_RECURSE tmp_cc ${folder}/*.cc)

	if (MSVC OR MINGW)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "stress_test"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		if (APPLE)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "@executable_path/../lib"
			)
		else ()
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "\$ORIGIN/../lib"
			)
		endif()
	endif(MSVC OR MINGW)
	add_dependencies(${name} ${muggle_c})
	target_include_directories(${name} PUBLIC
		${folder}
	)
	target_link_libraries(${name} ${muggle_c})

	target_compile_definitions(${name} PRIVATE MUGGLE_HOLD_LOG_MACRO)

	if (MUGGLE_INSTALL_BIN)
		install(TARGETS ${name} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
	endif()
endfunction()

function(add_benchmark name folder)
	message("add benchmark ${name} ${folder}")
	
	set(name benchmark_${name})

	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB tmp_cc ${folder}/*.cc)

	if (MSVC OR MINGW)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "benchmark"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "\$ORIGIN/../lib"
		)
	endif(MSVC OR MINGW)
	add_dependencies(${name} muggle_benchmark)
	target_link_libraries(${name} muggle_benchmark)
	target_compile_definitions(${name} PRIVATE MUGGLE_HOLD_LOG_MACRO)

	if (MUGGLE_INSTALL_BIN)
		install(TARGETS ${name} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
	endif()
endfunction()

function(add_gtest name folder)
	message("add test ${name} ${folder}")
	
	set(name test_${name})

	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB tmp_cc ${folder}/*.cc)

	if (MSVC OR MINGW)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "test"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "\$ORIGIN/../lib"
		)
	endif(MSVC OR MINGW)

	add_dependencies(${name} ${muggle_c} ${test_utils})
	if (FOUND_GTEST_FROM_DOWNLOAD)
		add_dependencies(${name} ${GTEST_BOTH_LIBRARIES})
	endif()
	target_include_directories(${name} PUBLIC
		${GTEST_INCLUDE_DIRS}
	)
	target_link_libraries(${name}
		${muggle_c}
		${test_utils}
		${GTEST_BOTH_LIBRARIES}
	)

	if (APPLE)
		# NOTE: APPLE not support TIME_UTC until c11 or c++17
		set_target_properties(${name} PROPERTIES
			CXX_STANDARD 17
			CXX_STANDARD_REQUIRED ON
		)
	else()
		set_target_properties(${name} PROPERTIES
			CXX_STANDARD 11
			CXX_STANDARD_REQUIRED ON
		)
	endif()

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
				PRIVATE MUGGLE_TEST_LINK_OPENSSL
			)
		endif()
	endif()

	target_compile_definitions(${name} PRIVATE MUGGLE_HOLD_LOG_MACRO)

	add_test(NAME ${name} COMMAND ${name})

	if (MUGGLE_INSTALL_BIN)
		install(TARGETS ${name} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
	endif()
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
		configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/googletest.cmake.in googletest-download/CMakeLists.txt)
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
		set(GTEST_BOTH_LIBRARIES gtest_main gtest)
		set(FOUND_GTEST_FROM_DOWNLOAD ON)

		message("-- Find GTest - use gtest from download")
	endif()

	if (MUGGLE_INSTALL_BIN)
		if (FOUND_GTEST_FROM_DOWNLOAD)
			install(TARGETS ${GTEST_BOTH_LIBRARIES}
				RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
				ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
				LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
		else()
			install(IMPORTED_RUNTIME_ARTIFACTS GTest::gtest GTest::gtest_main)
		endif()
	endif()

endif()

# example
if (${MUGGLE_BUILD_EXAMPLE})
	message("---------------------- examples ----------------------")
	SUBDIRLIST(example_dirs ${CMAKE_CURRENT_LIST_DIR}/examples/src)
	FOREACH(example_dir ${example_dirs})
		message("find example section directory: ${example_dir}")
		SUBDIRLIST(sub_example_dirs ${CMAKE_CURRENT_LIST_DIR}/examples/src/${example_dir})
		FOREACH(sub_example_dir ${sub_example_dirs})
			add_example(${example_dir}_${sub_example_dir}
				${CMAKE_CURRENT_LIST_DIR}/examples/src/${example_dir}/${sub_example_dir})
		ENDFOREACH()
	ENDFOREACH()
endif()

if (${MUGGLE_BUILD_STRESS_TEST})
	message("---------------------- stress tests ----------------------")
	SUBDIRLIST(stress_test_root_dir ${CMAKE_CURRENT_LIST_DIR}/stress_tests)
	FOREACH(subdir ${stress_test_root_dir})
		add_stress_test(${subdir} ${CMAKE_CURRENT_LIST_DIR}/stress_tests/${subdir})
	ENDFOREACH()
endif()

# test
if (${BUILD_TESTING})
	message("---------------------- test ----------------------")
	SUBDIRLIST(test_root_dir ${CMAKE_CURRENT_LIST_DIR}/test)
	FOREACH(subdir ${test_root_dir})
		add_gtest(${subdir} ${CMAKE_CURRENT_LIST_DIR}/test/${subdir})
	ENDFOREACH()
endif()

# benchmark
if (${MUGGLE_BUILD_BENCHMARK})
	message("---------------------- benchmark ----------------------")
	SUBDIRLIST(benchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/benchmark)
	FOREACH(subdir ${benchmark_root_dir})
		add_benchmark(${subdir} ${CMAKE_CURRENT_LIST_DIR}/benchmark/${subdir})
	ENDFOREACH()
endif()

# coverage
