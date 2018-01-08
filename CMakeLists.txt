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
target_link_libraries(${muggle_cc}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
)

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

if (${MUGGLE_BUILD_EXAMPLE} AND NOT ${MUGGLE_BUILD_ONLY_C})
	add_subdirectory(example)
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

# if (${BUILD_MUGGLE_BASE_SEPARATE})
# 
# 	# base c
# 	if (WIN32)
# 		set(targetLinks "")
# 	else()
# 		set(targetLinks pthread dl)
# 	endif()
# 	addMuggleLib(${muggle_base_c} ON MUGGLE_BASE_C_EXPORTS base_c "${targetLinks}" "")
# 
# 	# utils c
# 	addMuggleLib(${muggle_utils_c} ON MUGGLE_UTILS_C_EXPORTS utils_c ${muggle_base_c} ${muggle_base_c})
# 
# 	# base cpp
# 	if (WIN32)
# 		set(targetLinks ${muggle_base_c})
# 	else()
# 		set(targetLinks pthread dl ${muggle_base_c})
# 	endif()
# 	addMuggleLib(${muggle_base_cpp} OFF MUGGLE_BASE_CPP_EXPORTS base_cpp "${targetLinks}" ${muggle_base_c})
# 
# 	# utils cpp
# 	set(targetLinks ${muggle_base_cpp})
# 	addMuggleLib(${muggle_utils_cpp} OFF MUGGLE_UTILS_CPP_EXPORTS utils_cpp "${targetLinks}" ${muggle_base_cpp})
# 
# 	# set folder in vs
# 	set_target_properties(
# 		${muggle_base_c} ${muggle_base_cpp} ${muggle_utils_c} ${muggle_utils_cpp}
# 		PROPERTIES
# 		FOLDER "muggle"
# 	)
# 
# else()
# 	if (WIN32)
# 		set(targetLinks "")
# 	else()
# 		set(targetLinks pthread dl)
# 	endif()
# 
# 	list(APPEND private_exports_macros "MUGGLE_BASE_C_EXPORTS")
# 	list(APPEND private_exports_macros "MUGGLE_UTILS_C_EXPORTS")
# 	list(APPEND private_exports_macros "MUGGLE_BASE_CPP_EXPORTS")
# 	list(APPEND private_exports_macros "MUGGLE_UTILS_CPP_EXPORTS")
# 
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/c/base)
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_c)
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_c/time)
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/base_cpp)
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp)
# 
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp/time)
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp/mem_detect)
# 	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp/design_pattern)
# 
# 	# add project
# 	muggle_add_project_with_folders(${muggle_cc} ${CMAKE_CURRENT_LIST_DIR}/muggle ${MUGGLE_LIB_TYPE} "${dirs}")
# 
# 	# link libs
# 	if (WIN32)
# 		set(targetLinks "")
# 	else()
# 		set(targetLinks pthread dl)
# 	endif()
# 	target_link_libraries(${muggle_cc} ${targetLinks})
# 
# 	if(MSVC AND ${MUGGLE_BUILD_SHARED_LIB})
# 		target_compile_definitions(${muggle_cc}
# 			PUBLIC MUGGLE_USE_DLLS 
# 			PRIVATE ${private_exports_macros}
# 		)
# 	endif()
# 
# 	target_compile_definitions(${muggle_cc}
# 		PUBLIC ${public_exports_macros}
# 	)
# 
# 	# instal binary files
# 	install(TARGETS ${muggle_cc}
# 		RUNTIME DESTINATION ${MUGGLE_INSTALL_RUNTIME_PATH}
# 		LIBRARY DESTINATION ${MUGGLE_INSTALL_LIBRARY_PATH}
# 		ARCHIVE DESTINATION ${MUGGLE_INSTALL_ARCHIVE_PATH}
# 	)
# 
# 	# set folder in vs
# 	set_target_properties(
# 		${muggle_cc}
# 		PROPERTIES
# 		FOLDER "muggle"
# 	)
# endif()
# 
# # install headers
# muggle_install_headers(${muggle_source_dir}/muggle ${MUGGLE_INSTALL_INCLUDE_PATH})
# 
# if (${BUILD_MUGGLE_EXAMPLE} AND ${BUILD_MUGGLE_BASE_SEPARATE})
# 	add_subdirectory(example)
# endif()
# 
# if (${BUILD_MUGGLE_TEST} AND ${BUILD_MUGGLE_BASE_SEPARATE})
# 	# find googletest source
# 	include(${CMAKE_CURRENT_LIST_DIR}/cmake/FindGTestSrc.cmake)
# 	if(GTESTSRC_FOUND)
# 	
# 		if (WIN32)
# 			# without this, will cause error: Debug Assertion Failed! Expression: __acrt_first_block == header
# 			SET(BUILD_SHARED_LIBS ON CACHE BOOL "build google test as dynamic lib")
# 		endif()
# 		
# 		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/gtest)
# 		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
# 
# 		set_target_properties(
# 			gtest gtest_main gmock gmock_main
# 			PROPERTIES
# 			FOLDER "thirdparty"
# 		)
# 	endif()
# endif()
