## mugglec
<b>Cross platform c base library</b>
* [readme EN](./README.md)
* [readme 中文](./README_cn.md)

mugglec is a cross platform pure C base library, contains utilities like basic data structure, network, concurrency, log, crypt, memory pool, path operation and so on.

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
