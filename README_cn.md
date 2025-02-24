## mugglec

![linux-gcc](https://github.com/MuggleWei/mugglec/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/mugglec/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/mugglec/actions/workflows/win-msvc.yaml/badge.svg?branch=master)
![macos-clang](https://github.com/MuggleWei/mugglec/actions/workflows/macos-clang.yaml/badge.svg?branch=master)
![android](https://github.com/MuggleWei/mugglec/actions/workflows/android-cmake.yaml/badge.svg?branch=master)

<p align="center"><b>一个跨平台 C 语言基础库</b></p>
<p align="center"><a href="./README.md">English</a> | 中文</p>

mugglec是一个c语言编写, 跨平台基础库, 提供了一系列常用的功能, 比如基础的数据结构, 网络, 并发, 日志, 密码学, 内存池, 常用路径操作等等

### 目录

- [mugglec](#mugglec)
  - [目录](#目录)
  - [特点](#特点)
  - [构建](#构建)
  - [教程和示例](#教程和示例)
  - [接口文档](#接口文档)

### 特点
* 提供常用的基础功能
* 跨平台
* 无第三方依赖
* 易于使用
* 尽可能的保持体积小

### 构建
可以很轻松的通过cmake构建此库, 通常要做的就是:
```
mkdir build
cd build
cmake ..
```

默认情况下, 只会编译出单独的库, 也可以通过改变cmake的option来决定编译动态库或者是静态库, 是否编译例子, 单元测试以及性能测试, 下面列出了可供改变的cmake option
| option | default | desc |
| ---- | ---- | ---- |
| BUILD_SHARED_LIBS | ON | ON - 构建动态库, OFF - 构建静态库 |
| MUGGLE_BUILD_STATIC_PIC | ON | 若是静态库, 是否设置位置无关代码的标志 |
| BUILD_TESTING | OFF | 构建单元测试 |
| MUGGLE_BUILD_TRACE | OFF | 当构建的是Debug时, 是否增加一些额外的调试信息 |
| MUGGLE_BUILD_SANITIZER | OFF | 构建时增加 sanitizer 的编译选项 |
| MUGGLE_BUILD_EXAMPLE | OFF | 构建例子 |
| MUGGLE_BUILD_BENCHMARK_UTILS | OFF | 构建性能测试工具 |
| MUGGLE_BUILD_BENCHMARK | OFF | 构建性能测试 |
| MUGGLE_INSTALL_BIN | OFF | 安装时连例子、单元测试和性能测试的二进制文件一同安装 |

NOTE: 本库的单元测试使用的是 gtest, 若 BUILD_TESTING 被设置为ON, 构建时会首先寻找 gtest 库, 若在 CMAKE_PREFIX_PATH 的路径中没有发现 gtest, 则会在第一次构建时, 自动下载 gtest 到构建目录中

### 教程和示例
可以通过查看 [examples](./examples/readme_cn.md) 文件夹来发现教程与使用示例.  
当 cmake 构建时开启了 `MUGGLE_BUILD_EXAMPLE` 选项时, 则 [examples](./examples/readme_cn.md) 文件夹中的程序将被纳入工程构建当中. 此外, 该文件夹中包含一个独立的 CMakeLists.txt 文件, 如果用户想要体验一下在工程中引入 mugglec, 可以将 [examples](./examples/readme_cn.md) 文件夹拷贝到任意地点并进行构建, 它将在构建过程中, 自动下载 mugglec 的代码.  

### 接口文档
在工程根目录中有 Doxyfile, 用户可安装 [doxygen](https://doxygen.nl/) 后, 运行根目录中的 `gen_doxygen_doc.sh` 脚本, 生成项目文档