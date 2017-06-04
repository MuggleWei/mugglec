cmake_minimum_required (VERSION 2.8.9)
project(muggleCC)

# cmake
set(muggle_source_dir ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/cmake/muggle_cmake_common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/add_project.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/muggle_utils.cmake)

option(BUILD_MUGGLE_BASE_SEPARATE "Build muggle lib separately" ON)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# options for build example and test
option(BUILD_MUGGLE_EXAMPLE "Build example" ON)
option(BUILD_MUGGLE_TEST "Build test" OFF)

# options for component in muggleCC
option(BUILD_MUGGLE_UTILS_CPP_TIME "Build muggle utils time" ON)
option(BUILD_MUGGLE_UTILS_CPP_MEMORY_DETECT "Build muggle utils memory detect" ON)

if (${BUILD_MUGGLE_BASE_SEPARATE})

	################## essential component ##################

	# base c
	if (WIN32)
		set(targetLinks "")
	else()
		set(targetLinks pthread dl)
	endif()
	addMuggleLib(${muggle_base_c} ON MUGGLE_BASE_C_EXPORTS base_c "${targetLinks}" "")

	# utils c
	addMuggleLib(${muggle_utils_c} ON MUGGLE_UTILS_C_EXPORTS utils_c ${muggle_base_c} ${muggle_base_c})

	# base cpp
	if (WIN32)
		set(targetLinks ${muggle_base_c})
	else()
		set(targetLinks pthread dl ${muggle_base_c})
	endif()
	addMuggleLib(${muggle_base_cpp} OFF MUGGLE_BASE_CPP_EXPORTS base_cpp "${targetLinks}" ${muggle_base_c})

	# set folder in vs
	set_target_properties(
		${muggle_base_c} ${muggle_base_cpp} ${muggle_utils_c}
		PROPERTIES
		FOLDER "muggle"
	)

	################## option component ##################

	set(muggle_utils_cpp_folders "")

	# time
	if (BUILD_MUGGLE_UTILS_CPP_TIME)
		list(APPEND muggle_utils_cpp_folders ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp/time)
	endif()

	# memory detect
	if (BUILD_MUGGLE_UTILS_CPP_MEMORY_DETECT)
		list(APPEND muggle_utils_cpp_folders ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp/mem_detect)
	endif()

	if (NOT "${muggle_utils_cpp_folders}")
		list(APPEND muggle_utils_cpp_folders ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp)
		muggle_add_project_with_folders(${muggle_utils_cpp} ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp SHARED "${muggle_utils_cpp_folders}")

		# link
		target_link_libraries(${muggle_utils_cpp}
			${muggle_base_cpp}
		)
		add_dependencies(${muggle_utils_cpp}
			${muggle_base_cpp}
		)

		# macro
		if (BUILD_MUGGLE_UTILS_CPP_TIME)
			target_compile_definitions(${muggle_utils_cpp}
				PUBLIC USE_MUGGLE_TIME
			)
		endif()
		if (BUILD_MUGGLE_UTILS_CPP_MEMORY_DETECT)
			target_compile_definitions(${muggle_utils_cpp}
				PUBLIC USE_MUGGLE_MEMORY_DETECT
			)
		endif()

		# windows dll export and import
		if(MSVC AND ${MUGGLE_BUILD_SHARED_LIB})
			target_compile_definitions(${muggle_utils_cpp}
				PUBLIC MUGGLE_USE_DLLS
				PRIVATE MUGGLE_UTILS_CPP_EXPORTS
			)
		endif()

		# instal binary files
		install(TARGETS ${muggle_utils_cpp}
			RUNTIME DESTINATION ${MUGGLE_INSTALL_RUNTIME_PATH}
			LIBRARY DESTINATION ${MUGGLE_INSTALL_LIBRARY_PATH}
			ARCHIVE DESTINATION ${MUGGLE_INSTALL_ARCHIVE_PATH}
		)

		# set folder in vs
		set_target_properties(
			${muggle_utils_cpp}
			PROPERTIES
			FOLDER "muggle"
		)
	endif()	

else()
	if (WIN32)
		set(targetLinks "")
	else()
		set(targetLinks pthread dl)
	endif()

	list(APPEND private_exports_macros "MUGGLE_BASE_C_EXPORTS")
	list(APPEND private_exports_macros "MUGGLE_UTILS_C_EXPORTS")
	list(APPEND private_exports_macros "MUGGLE_BASE_CPP_EXPORTS")
	list(APPEND private_exports_macros "MUGGLE_UTILS_CPP_EXPORTS")

	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/base_c)
	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_c)
	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_c/time)
	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/base_cpp)
	list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp)

	if (BUILD_MUGGLE_UTILS_CPP_TIME)
		list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp/time)
		list(APPEND public_exports_macros USE_MUGGLE_TIME)
	endif()

	if (BUILD_MUGGLE_UTILS_CPP_MEMORY_DETECT)
		list(APPEND dirs ${CMAKE_CURRENT_LIST_DIR}/muggle/utils_cpp/mem_detect)
		list(APPEND public_exports_macros USE_MUGGLE_MEMORY_DETECT)
	endif()

	# add project
	muggle_add_project_with_folders(${muggle_cc} ${CMAKE_CURRENT_LIST_DIR}/muggle SHARED "${dirs}")

	# link libs
	if (WIN32)
		set(targetLinks "")
	else()
		set(targetLinks pthread dl)
	endif()
	target_link_libraries(${muggle_cc} ${targetLinks})

	if(MSVC AND ${MUGGLE_BUILD_SHARED_LIB})
		target_compile_definitions(${muggle_cc}
			PUBLIC MUGGLE_USE_DLLS 
			PRIVATE ${private_exports_macros}
		)
	endif()

	target_compile_definitions(${muggle_cc}
		PUBLIC ${public_exports_macros}
	)

	# instal binary files
	install(TARGETS ${muggle_cc}
		RUNTIME DESTINATION ${MUGGLE_INSTALL_RUNTIME_PATH}
		LIBRARY DESTINATION ${MUGGLE_INSTALL_LIBRARY_PATH}
		ARCHIVE DESTINATION ${MUGGLE_INSTALL_ARCHIVE_PATH}
	)

	# set folder in vs
	set_target_properties(
		${muggle_cc}
		PROPERTIES
		FOLDER "muggle"
	)
endif()

# install headers
muggle_install_headers(${muggle_source_dir}/muggle ${MUGGLE_INSTALL_INCLUDE_PATH})

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
