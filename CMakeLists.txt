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
option(MUGGLE_BUILD_SHARED_LIB ON)
option(MUGGLE_BUILD_EXAMPLE "Build example" OFF)
option(BUILD_TESTING "Build test" OFF)
option(BUILD_BENCHMARK "Build benchmark" OFF)


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

	muggle_add_project(muggle_benchmark ${CMAKE_CURRENT_LIST_DIR}/muggle_benchmark STATIC)
	target_link_libraries(muggle_benchmark ${muggle_cc})
	add_dependencies(muggle_benchmark ${muggle_cc})
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
	add_dependencies(${name} ${muggle_cc})
	target_link_libraries(${name}
		${muggle_cc}
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
	add_dependencies(${name} ${muggle_cc} gtest_main)
	target_link_libraries(${name}
		${muggle_cc}
		gtest_main
	)
	add_test(NAME ${name} COMMAND ${name})
endfunction()

# example
if (${MUGGLE_BUILD_EXAMPLE} AND NOT ${MUGGLE_BUILD_ONLY_C})
	SUBDIRLIST(example_dirs ${CMAKE_CURRENT_LIST_DIR}/example)
	FOREACH(subdir ${example_dirs})
		add_example(${subdir} ${CMAKE_CURRENT_LIST_DIR}/example/${subdir})
	ENDFOREACH()
endif()


# test
if (BUILD_TESTING AND NOT ${MUGGLE_BUILD_ONLY_C})
	enable_testing()

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


	SUBDIRLIST(test_root_dir ${CMAKE_CURRENT_LIST_DIR}/test)
	FOREACH(subdir ${test_root_dir})
		add_gtest(${subdir} ${CMAKE_CURRENT_LIST_DIR}/test/${subdir})
	ENDFOREACH()
endif()

# benchmark
if (BUILD_BENCHMARK AND NOT ${MUGGLE_BUILD_ONLY_C})
	SUBDIRLIST(benchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/benchmark)
	FOREACH(subdir ${benchmark_root_dir})
		add_benchmark(${subdir} ${CMAKE_CURRENT_LIST_DIR}/benchmark/${subdir})
	ENDFOREACH()
endif()
