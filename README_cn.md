## mugglec
<b>跨平台c语言基础库</b>
* [readme EN](./README.md)
* [readme 中文](./README_cn.md)


mugglec是一个纯c语言编写, 跨平台基础库, 提供了一系列常用的功能, 比如基础的数据结构, 网络, 并发, 日志, 密码学, 内存池, 常用路径操作等等

### 特点
* 提供常用的基础功能
* 跨平台, Linux/Windows(主要目标)
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
| MUGGLE_BUILD_SHARED_LIB | ON | ON - 构建动态库, OFF - 构建静态库 |
| MUGGLE_BUILD_STATIC_PIC | ON | 若是静态库, 是否设置位置无关代码的标志 |
| MUGGLE_BUILD_EXAMPLE | OFF | 构建例子 |
| MUGGLE_BUILD_TESTING | OFF | 构建单元测试 |
| MUGGLE_BUILD_BENCHMARK | OFF | 构建性能测试 |
| MUGGLE_BUILD_TRACE | OFF | 当构建的是Debug时, 是否增加一些额外的调试信息 |
| MUGGLE_EXTRA_PREFIX_PATH | "" | 指定寻找库函数的路径, 它将会被添加到CMAKE_PREFIX_PATH |

NOTE: 本库的单元测试使用的是gtest, 若MUGGLE_BUILD_TESTING被设置为ON, 构建时会首先寻找gtest库, 若在CMAKE_PREFIX_PATH的路径中没有发现gtest, 则会在第一次构建时, 自动下载gtest到构建目录中