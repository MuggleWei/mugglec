# set compile parameter
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
endif()

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set install variables
set(MUGGLE_INSTALL_RUNTIME_PATH bin/muggle)
set(MUGGLE_INSTALL_LIBRARY_PATH lib/muggle)
set(MUGGLE_INSTALL_ARCHIVE_PATH lib/muggle)
set(MUGGLE_INSTALL_INCLUDE_PATH include/muggle)

# set rpath
set(CMAKE_INSTALL_RPATH "$ORIGIN/../../lib/muggle:$ORIGIN:${CMAKE_INSTALL_PREFIX}/lib")

# architecture 32bit or 64bit
set(MG_ARCHITECTURE "")
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(MUGGLE_ARCHITECTURE x64)
else()
	set(MUGGLE_ARCHITECTURE x86)
endif()

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# build share lib or static lib
set(MUGGLE_BUILD_SHARED_LIB ON)
if (MUGGLE_BUILD_SHARED_LIB)
	set(MUGGLE_LIB_TYPE SHARED)
else()
	set(MUGGLE_LIB_TYPE STATIC)
endif()

# include directories
include_directories(
	${muggle_source_dir}
)

# set variables
set(muggle_base_c muggleBaseC)
set(muggle_base_cpp muggleBaseCpp)