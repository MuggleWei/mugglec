## mugglec

![linux-gcc](https://github.com/MuggleWei/mugglec/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/mugglec/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/mugglec/actions/workflows/win-msvc.yaml/badge.svg?branch=master)
![macos-clang](https://github.com/MuggleWei/mugglec/actions/workflows/macos-clang.yaml/badge.svg?branch=master)

<b>Cross platform c base library</b>
* [readme EN](./README.md)
* [readme 中文](./README_cn.md)

mugglec is a cross platform C base library, contains utilities like basic data structure, network, concurrency, log, crypt, memory pool, path operation and so on.

### Table of content
- [mugglec](#mugglec)
  - [Table of content](#table-of-content)
  - [Feature](#feature)
  - [Build](#build)
  - [tutorial and examples](#tutorial-and-examples)
  - [doc](#doc)
  - [import mugglec library](#import-mugglec-library)
    - [Incorporating Into CMake project(recommended)](#incorporating-into-cmake-projectrecommended)
      - [current style](#current-style)
      - [old style](#old-style)
    - [Use find\_package or pkg-config (recommended)](#use-find_package-or-pkg-config-recommended)
      - [find\_package](#find_package)
      - [pkg-config](#pkg-config)
    - [Search and link](#search-and-link)
    - [Use git submodule(Not recommended)](#use-git-submodulenot-recommended)

### Feature
* Provide common basic functions
* Cross platform
* No dependencies
* Easy to use
* As tiny as possible

### Build
It's easy to build this project with CMake, the typical workflow starts with:
```
mkdir build
cd build
cmake ..
```

There are some cmake build options to decide build this library as shared or static, whether build example, test and benchmark, and add some extra library search path.  
| option | default | desc |
| ---- | ---- | ---- |
| BUILD_SHARED_LIBS | ON | build shared library |
| MUGGLE_BUILD_STATIC_PIC | ON | if build static library, set position independent code flag |
| MUGGLE_BUILD_TRACE | OFF | build with trace info in debug |
| BUILD_TESTING | OFF | build unittests |
| MUGGLE_BUILD_EXAMPLE | OFF | build examples |
| MUGGLE_BUILD_BENCHMARK | OFF | build benchmark |
| MUGGLE_INSTALL_BIN | OFF | install example, unittest and benchmark binaries |

NOTE: mugglec unittest use gtest, so if BUILD_TESTING is ON, it will try find gtest first, if gtest not found, will download gtest in first time automaticlly.  

### tutorial and examples
See [examples](./examples/readme_cn.md) to find tutorial and examples.  
If cmake build with option `MUGGLE_BUILD_EXAMPLE` ON, then [examples](./examples/readme_cn.md) folder will be build automatically.  

### doc
There are `Doxyfile` in the project root directory, user can install [doxygen](https://doxygen.nl/), and run `gen_doxygen_doc.sh` that in the project root directory to generate project doc.   

### import mugglec library
If you want to import mugglec into your project, there are several methods: 

#### Incorporating Into CMake project(recommended)
If your project already use CMake, then you can invoked mugglec as a sub-build during the CMake stage.  

##### current style
If your cmake version >= 3.11, you can use ```FetchContent``` directly, add content below into your CMakeLists.txt. Then when run ```cmake ..```, cmake will download mugglec into ```${FETCHCONTENT_BASE_DIR}``` directory.
```
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# set mugglec compile options
set(MUGGLE_BUILD_TRACE OFF CACHE BOOL "")
set(MUGGLE_BUILD_SHARED_LIB ON CACHE BOOL "")
set(MUGGLE_BUILD_STATIC_PIC ON CACHE BOOL "")
set(MUGGLE_BUILD_BENCHMARK OFF CACHE BOOL "")
set(MUGGLE_BUILD_TESTING OFF CACHE BOOL "")
set(MUGGLE_BUILD_EXAMPLE OFF CACHE BOOL "")

FetchContent_Declare(
	mugglec
	GIT_REPOSITORY https://github.com/MuggleWei/mugglec.git
	GIT_TAG v1.2.5
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(mugglec)

# link mugglec
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
target_include_directories(example PUBLIC
	${FETCHCONTENT_BASE_DIR}/mugglec-src
	${FETCHCONTENT_BASE_DIR}/mugglec-build/generated)
```

##### old style
If your cmake version < 3.11, don't worry, we have old style method to implement download action.  
Create new file cmake/mugglec.cmake.in
```
cmake_minimum_required(VERSION 3.0.2)

project(mugglec-download NONE)

include(ExternalProject)
ExternalProject_Add(mugglec
        GIT_REPOSITORY    https://github.com/MuggleWei/mugglec.git
        GIT_TAG           v1.2.5
        GIT_SHALLOW       TRUE
        SOURCE_DIR        "${FETCHCONTENT_BASE_DIR}/mugglec-src"
        BINARY_DIR        "${FETCHCONTENT_BASE_DIR}/mugglec-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
        TEST_COMMAND      ""
)
```

add content below into your CMakeLists.txt
```
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# Download and unpack mugglec at configure time
configure_file(
	${CMAKE_CURRENT_LIST_DIR}/cmake/mugglec.cmake.in
	${FETCHCONTENT_BASE_DIR}/mugglec-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${FETCHCONTENT_BASE_DIR}/mugglec-download)
if (result)
	message(FATAL_ERROR "cmake step for mugglec failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${FETCHCONTENT_BASE_DIR}/mugglec-download)
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
	${FETCHCONTENT_BASE_DIR}/mugglec-src
	${FETCHCONTENT_BASE_DIR}/mugglec-build)

# link mugglec and include header files
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
target_include_directories(example PUBLIC
	${FETCHCONTENT_BASE_DIR}/mugglec-src
	${FETCHCONTENT_BASE_DIR}/mugglec-build/generated)
```

#### Use find_package or pkg-config (recommended)
If you don't want to compile mugglec in each project, you can compile and install once, after just link it.  
After build and install(`cmake --build . --target install`) mugglec, you can find and link it.  

##### find_package
If your project already use CMake, add content below into your CMakeLists.txt  
```
# find mugglec package
find_package(mugglec 1 REQUIRED)

# link mugglec (NOTE: no need to include directory manually)
add_executable(example src/example.c)
target_link_libraries(example PUBLIC mugglec)
```

##### pkg-config
If your project is not built with cmake, it will most likely support pkg-config.  
For example, when using meson, you can easily add dependencies through pkg-config
* Specify the path where the pkg-config dir is
  ```
  export PKG_CONFIG_PATH=/wherever/your/pkgconfig/dir/is/
  ```
* Modify meson.build, add mugglec dependency
  ```
  mugglecdep = dependency('mugglec')
  executable('hello', 'hello.c', dependencies: mugglecdep)
  ```

#### Search and link
When you don't wanna use `find_package` or only have mugglec libraries and headers in your system, you can try to find `mugglec` manually
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

# link mugglec and include head directories
add_executable(example src/example.c)
target_link_libraries(example ${MUGGLEC_LIBRARIES})
target_include_directories(example PUBLIC ${MUGGLEC_INCLUDE_DIR})
```

#### Use git submodule(Not recommended)
In addition to the two methods mentioned above, import mugglec as git submodule is another option
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
