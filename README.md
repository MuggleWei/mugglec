## mugglec
<b>Cross platform c base library</b>
* [readme EN](./README.md)
* [readme 中文](./README_cn.md)

mugglec is a cross platform pure C base library, contains utilities like basic data structure, network, concurrency, log, crypt, memory pool, path operation and so on.

### Table of content
- [mugglec](#mugglec)
  - [Table of content](#table-of-content)
  - [Feature](#feature)
  - [Build](#build)
  - [import mugglec library](#import-mugglec-library)
    - [Incorporating Into CMake project(recommended)](#incorporating-into-cmake-projectrecommended)
    - [Find and link](#find-and-link)
    - [Use git submodule](#use-git-submodule)

### Feature
* Provide common basic functions
* Cross platform (mostly), Linux/Windows(main target)
* No dependencies
* Easy to use
* As tiny as possible

### Build
![linux-gcc](https://github.com/MuggleWei/mugglec/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/mugglec/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/mugglec/actions/workflows/win-msvc.yaml/badge.svg?branch=master)

It's easy to build this project with CMake, the typical workflow starts with:
```
mkdir build
cd build
cmake ..
```

There are some cmake build options to decide build this library as shared or static, whether build example, test and benchmark, and add some extra library search path.  
| option | default | desc |
| ---- | ---- | ---- |
| MUGGLE_BUILD_SHARED_LIB | ON | build shared library |
| MUGGLE_BUILD_STATIC_PIC | ON | if build static library, set position independent code flag |
| MUGGLE_BUILD_EXAMPLE | OFF | build examples |
| MUGGLE_BUILD_TESTING | OFF | build unittests |
| MUGGLE_BUILD_BENCHMARK | OFF | build benchmark |
| MUGGLE_BUILD_TRACE | OFF | build with trace info in debug |
| MUGGLE_EXTRA_PREFIX_PATH | "" | specify extra find path, it will append into CMAKE_PREFIX_PATH |

NOTE: mugglec unittest use gtest, so if MUGGLE_BUILD_TESTING is ON, it will try find gtest first, if gtest not found, will download gtest in first time automaticlly.  

### import mugglec library
If you want to import mugglec into your project, there are several methods: 

#### Incorporating Into CMake project(recommended)
If your project already use CMake, then you can invoked mugglec as a sub-build during the CMake stage.  
New file cmake/mugglec.cmake.in
```
cmake_minimum_required(VERSION 3.0.2)

project(mugglec-download NONE)

include(ExternalProject)
ExternalProject_Add(mugglec
        GIT_REPOSITORY    https://github.com/MuggleWei/mugglec.git
        # GIT_TAG         master
        # GIT_SHALLOW     TRUE
        # specify a commit
        GIT_TAG           fd059d4100ee59c55a63d2c84ed045a5a757933d
        SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/mugglec-src"
        BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/mugglec-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
        TEST_COMMAND      ""
)
```

add content below into your CMakeLists.txt
```
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
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/mugglec-src
        ${CMAKE_CURRENT_BINARY_DIR}/mugglec-build)

# include mugglec head directories
include_directories(${CMAKE_CURRENT_BINARY_DIR}/mugglec-src)

# link mugglec
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
```

#### Find and link
If you don't want to compile mugglec in each project, you can compile and install once, after just link it.  
After build and install mugglec, you can find and link it.  
If your project already use CMake, add content below into your CMakeLists.txt  
```
# search head files and libraries
find_path(MUGGLEC_INCLUDE_DIR
        NAMES muggle/c/muggle_c.h
        PATH_SUFFIXES include)
find_library(MUGGLEC_LIBRARIES
        NAMES mugglec
        PATH_SUFFIXES lib)

# check head files and libraries exists
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MUGGLEC
        REQUIRED_VARS MUGGLEC_INCLUDE_DIR MUGGLEC_LIBRARIES
)
if (MUGGLEC_FOUND)
        message("find mugglec include dir: ${MUGGLEC_INCLUDE_DIR}")
        message("find mugglec libraries: ${MUGGLEC_LIBRARIES}")
else()
        message(FATAL_ERROR "failed found mugglec")
endif()

# include mugglec head directories
include_directories(${MUGGLEC_INCLUDE_DIR})

# link mugglec
add_executable(example src/example.c)
target_link_libraries(example ${MUGGLEC_LIBRARIES})
```

#### Use git submodule
Invoke mugglec as git submodule
```
git submodule add https://github.com/MuggleWei/mugglec.git thirdparty/mugglec
```

If your project already use CMake, add content below into your CMakeLists.txt
```
# add mugglec to build
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/mugglec)

# include mugglec head directories
include_directories(${CMAKE_CURRENT_LIST_DIR}/thirdparty/mugglec)

# link mugglec
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
```
