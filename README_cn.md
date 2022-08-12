## mugglec
<b>跨平台c语言基础库</b>
* [readme EN](./README.md)
* [readme 中文](./README_cn.md)


mugglec是一个c语言编写, 跨平台基础库, 提供了一系列常用的功能, 比如基础的数据结构, 网络, 并发, 日志, 密码学, 内存池, 常用路径操作等等

### 目录

- [mugglec](#mugglec)
  - [目录](#目录)
  - [特点](#特点)
  - [构建](#构建)
  - [教程和示例](#教程和示例)
  - [引入mugglec](#引入mugglec)
    - [融入进CMake工程 (推荐)](#融入进cmake工程-推荐)
      - [当前的风格](#当前的风格)
      - [老式的风格](#老式的风格)
    - [发现并链接](#发现并链接)
    - [使用git子模块(不推荐)](#使用git子模块不推荐)

### 特点
* 提供常用的基础功能
* 跨平台
* 无第三方依赖
* 易于使用
* 尽可能的保持体积小

### 构建
![linux-gcc](https://github.com/MuggleWei/mugglec/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/mugglec/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/mugglec/actions/workflows/win-msvc.yaml/badge.svg?branch=master)
![macos-clang](https://github.com/MuggleWei/mugglec/actions/workflows/macos-clang.yaml/badge.svg?branch=master)

可以很轻松的通过cmake构建此库, 通常要做的就是:
```
mkdir build
cd build
cmake ..
```

默认情况下, 只会编译出单独的库, 也可以通过改变cmake的option来决定编译动态库或者是静态库, 是否编译例子, 单元测试以及性能测试, 下面列出了可供改变的cmake option
| option | default | desc |
| ---- | ---- | ---- |
| MUGGLE_BUILD_SHARED_LIB | ON | ON - 构建动态库, OFF - 构建静态库 |
| MUGGLE_BUILD_STATIC_PIC | ON | 若是静态库, 是否设置位置无关代码的标志 |
| MUGGLE_BUILD_EXAMPLE | OFF | 构建例子 |
| MUGGLE_BUILD_TESTING | OFF | 构建单元测试 |
| MUGGLE_BUILD_BENCHMARK | OFF | 构建性能测试 |
| MUGGLE_BUILD_TRACE | OFF | 当构建的是Debug时, 是否增加一些额外的调试信息 |
| MUGGLE_EXTRA_PREFIX_PATH | "" | 指定寻找库函数的路径, 它将会被添加到CMAKE_PREFIX_PATH |

NOTE: 本库的单元测试使用的是gtest, 若MUGGLE_BUILD_TESTING被设置为ON, 构建时会首先寻找gtest库, 若在CMAKE_PREFIX_PATH的路径中没有发现gtest, 则会在第一次构建时, 自动下载gtest到构建目录中

### 教程和示例
可以通过查看[examples](./examples/readme_cn.md)文件夹来发现教程与使用示例.  
当cmake构建时开启了`MUGGLE_BUILD_EXAMPLE`选项时, 则[examples](./examples/readme_cn.md)文件夹中的程序将被纳入工程构建当中. 此外, 该文件夹中包含一个独立的CMakeLists.txt文件, 如果用户想要体验一下在工程中引入mugglec, 可以将[examples](./examples/readme_cn.md)文件夹拷贝到任意地点并进行构建, 它将在构建过程中, 自动下载mugglec的代码.  

### 引入mugglec
想要在自己的工程中引入mugglec, 有几种普遍的做法

#### 融入进CMake工程 (推荐)
如果你的工程也使用CMake来构建，那么可以在 CMake 阶段调用 mugglec 作为子构建。

##### 当前的风格
若你的cmake版本 >= 3.11, 那么可以直接使用```FetchContent```, 将以下内容添加到你的CMakeLists.txt文件中, 当你运行```cmake ..```时, cmake将自动下载mugglec到```${FETCHCONTENT_BASE_DIR}```目录当中
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
        GIT_TAG v0.2.1
)
FetchContent_MakeAvailable(mugglec)

# link mugglec
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
target_include_directories(example PUBLIC
        ${FETCHCONTENT_BASE_DIR}/mugglec-src)
```

##### 老式的风格
如果你的cmake版本 < 3.11, 也无需担心, 我们也有老式的办法来实现同样的功能.  

在工程目录中新增文件 cmake/mugglec.cmake.in
```
cmake_minimum_required(VERSION 3.0.2)

project(mugglec-download NONE)

include(ExternalProject)
ExternalProject_Add(mugglec
        GIT_REPOSITORY    https://github.com/MuggleWei/mugglec.git
        GIT_TAG           v0.2.1
        GIT_SHALLOW       TRUE
        SOURCE_DIR        "${CMAKE_BINARY_DIR}/_deps/mugglec-src"
        BINARY_DIR        "${CMAKE_BINARY_DIR}/_deps/mugglec-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
        TEST_COMMAND      ""
)
```

接着在工程的CMakeLists.txt中添加
```
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# 在配置阶段, 下载mugglec并解包
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

# 设置mugglec的编译变量, 可根据需要进行更改
set(MUGGLE_BUILD_TRACE OFF CACHE BOOL "")
set(MUGGLE_BUILD_SHARED_LIB ON CACHE BOOL "")
set(MUGGLE_BUILD_STATIC_PIC ON CACHE BOOL "")
set(MUGGLE_BUILD_BENCHMARK OFF CACHE BOOL "")
set(MUGGLE_BUILD_TESTING OFF CACHE BOOL "")
set(MUGGLE_BUILD_EXAMPLE OFF CACHE BOOL "")

# 将mugglec加入工程中
add_subdirectory(
	${FETCHCONTENT_BASE_DIR}/mugglec-src
	${FETCHCONTENT_BASE_DIR}/mugglec-build)

# 链接mugglec 并 包含mugglec的头文件路径
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
target_include_directories(example PUBLIC
	${FETCHCONTENT_BASE_DIR}/mugglec-src)
```

#### 发现并链接
若你不想每个工程都独立去编译mugglec, 可以选择只编译一次并安装, 之后只需链接即可.  
成功构建mugglec后, 执行install步骤, 成功之后便可在系统中查找mugglec  
若你的工程使用的是cmake, 可在CMakeLists.txt中添加
```
# 查找头文件和库
find_path(MUGGLEC_INCLUDE_DIR
        NAMES muggle/c/muggle_c.h
        PATH_SUFFIXES include)
find_library(MUGGLEC_LIBRARIES
        NAMES mugglec
        PATH_SUFFIXES lib)

# 检查头文件和库是否存在
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

# 包含mugglec的头文件路径
include_directories(${MUGGLEC_INCLUDE_DIR})

# 链接mugglec
add_executable(example src/example.c)
target_link_libraries(example ${MUGGLEC_LIBRARIES})
```

#### 使用git子模块(不推荐)
除了上面的两中方法外, 还可以将mugglec直接作为git子模块引入工程当中
```
git submodule add https://github.com/MuggleWei/mugglec.git thirdparty/mugglec
```

若你的工程使用的是cmake, 可在CMakeLists.txt中添加
```
# 将mugglec加入工程中
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/mugglec)

# 包含mugglec的头文件路径
include_directories(${CMAKE_CURRENT_LIST_DIR}/thirdparty/mugglec)

# 链接mugglec
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
```