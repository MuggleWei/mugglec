cmake_minimum_required (VERSION 3.0.2)
project(mugglec_doc_codes)

# set compile parameter
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
endif()

# set standard and print features
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################
# mugglec

# Download and unpack mugglec at configure time
configure_file(
	${CMAKE_CURRENT_LIST_DIR}/cmake/mugglec.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/mugglec-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mugglec-download)
if (result)
	message(FATAL_ERROR "cmake step for mugglec failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mugglec-download)
if (result)
	message(FATAL_ERROR "build step for mugglec failed: ${result}")
endif()

# set mugglec compile options
set(MUGGLE_BUILD_TRACE OFF CACHE BOOL "")
set(MUGGLE_BUILD_SHARED_LIB ON CACHE BOOL "")
set(MUGGLE_BUILD_STATIC_PIC ON CACHE BOOL "")
set(MUGGLE_BUILD_BENCHMARK OFF CACHE BOOL "")
set(MUGGLE_BUILD_TESTING OFF CACHE BOOL "")
set(MUGGLE_BUILD_EXAMPLE OFF CACHE BOOL "")

# add mugglec to build
add_subdirectory(
	${CMAKE_CURRENT_BINARY_DIR}/mugglec-src
    ${CMAKE_CURRENT_BINARY_DIR}/mugglec-build)

################################
# projects

# cmake
include(${CMAKE_CURRENT_BINARY_DIR}/mugglec-src/cmake/muggle_utils.cmake)

set(muggle_c mugglec)

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
	target_link_libraries(${name} ${muggle_c})
	add_dependencies(${name} ${muggle_c})
	target_include_directories(${name} PUBLIC
		${CMAKE_CURRENT_BINARY_DIR}/mugglec-src
	)
endfunction()

message("---------------------- network unix only ----------------------")
if (UNIX)
	SUBDIRLIST(example_dirs ${CMAKE_CURRENT_LIST_DIR}/codes/network_unix)
	FOREACH(subdir ${example_dirs})
		add_example(${subdir} ${CMAKE_CURRENT_LIST_DIR}/codes/network_unix/${subdir})
	ENDFOREACH()
endif()

message("---------------------- network simple ----------------------")
SUBDIRLIST(example_dirs ${CMAKE_CURRENT_LIST_DIR}/codes/network_simple)
FOREACH(subdir ${example_dirs})
	add_example(${subdir} ${CMAKE_CURRENT_LIST_DIR}/codes/network_simple/${subdir})
ENDFOREACH()

message("---------------------- network foo ----------------------")
function(add_example_net_foo ver folder)
	message("add example net foo_${ver} ${folder}")

	set(foo example_net_foo_${ver})
	set(foo_srv example_net_foo_srv_${ver})

	muggle_add_project(${foo} ${folder}/foo SHARED)
	target_link_libraries(${foo} ${muggle_c})
	add_dependencies(${foo} ${muggle_c})
	target_include_directories(${foo} PUBLIC
		${folder}
		${CMAKE_CURRENT_BINARY_DIR}/mugglec-src
	)
	target_compile_definitions(${foo}
		PUBLIC NET_FOO_USE_DLL
		PRIVATE NET_FOO_EXPORTS 
	)

	message("${folder}/foo_srv")
	muggle_add_project(${foo_srv} ${folder}/foo_srv EXE)
	target_link_libraries(${foo_srv} ${foo})
	add_dependencies(${foo_srv} ${foo})

	if (WIN32)
		set_target_properties(${foo} ${foo_srv}
			PROPERTIES
			FOLDER "example"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	endif(WIN32)
endfunction()

add_example_net_foo(1 ${CMAKE_CURRENT_LIST_DIR}/codes/network_foo_v1)
