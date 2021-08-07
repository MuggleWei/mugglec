## mugglec
<b>跨平台c语言基础库</b>
* [readme EN](./README.md)
* [readme 中文](./README_cn.md)


mugglec是一个纯c语言编写, 跨平台基础库, 提供了一系列常用的功能, 比如基础的数据结构, 网络, 并发, 日志, 密码学, 内存池, 常用路径操作等等

### 目录

- [mugglec](#mugglec)
  - [目录](#目录)
  - [特点](#特点)
  - [构建](#构建)
  - [使用mugglec](#使用mugglec)
    - [融入进CMake工程 (推荐)](#融入进cmake工程-推荐)
    - [发现并链接](#发现并链接)
    - [使用git子模块](#使用git子模块)

### 特点
* 提供常用的基础功能
* 跨平台, Linux/Windows(主要目标)
* 无第三方依赖
* 易于使用
* 尽可能的保持体积小

### 构建
![linux-gcc](https://github.com/MuggleWei/mugglec/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/mugglec/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/mugglec/actions/workflows/win-msvc.yaml/badge.svg?branch=master)

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

### 使用mugglec
如果你想要在自己的工程中使用mugglec, 有几种普遍的做法

#### 融入进CMake工程 (推荐)
若你的工程是使用cmake构建, 那么可以在工程目录中新增文件 cmake/mugglec.cmake.in
```
cmake_minimum_required(VERSION 3.0.2)

project(mugglec-download NONE)

include(ExternalProject)
ExternalProject_Add(mugglec
        GIT_REPOSITORY    https://github.com/MuggleWei/mugglec.git
        GIT_TAG           v0.0.1-alpha.2
        GIT_SHALLOW       TRUE
        SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/mugglec-src"
        BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/mugglec-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
        TEST_COMMAND      ""
)
```

接着在工程的CMakeLists.txt中添加
```
# 在配置阶段, 下载mugglec并解包
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

# 设置mugglec的编译变量, 可根据需要进行更改
set(MUGGLE_BUILD_TRACE OFF CACHE BOOL "")
set(MUGGLE_BUILD_SHARED_LIB ON CACHE BOOL "")
set(MUGGLE_BUILD_STATIC_PIC ON CACHE BOOL "")
set(MUGGLE_BUILD_BENCHMARK OFF CACHE BOOL "")
set(MUGGLE_BUILD_TESTING OFF CACHE BOOL "")
set(MUGGLE_BUILD_EXAMPLE OFF CACHE BOOL "")

# 将mugglec加入工程中
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/mugglec-src
        ${CMAKE_CURRENT_BINARY_DIR}/mugglec-build)

# 包含mugglec的头文件路径
include_directories(${CMAKE_CURRENT_BINARY_DIR}/mugglec-src)

# 链接mugglec
add_executable(example src/example.c)
add_dependencies(example mugglec)
target_link_libraries(example mugglec)
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


#### 使用git子模块
可以将mugglec直接作为git子模块引入工程当中, 之后在构建工具中引入mugglec
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
