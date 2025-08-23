## mugglec

![linux-gcc](https://github.com/MuggleWei/mugglec/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/mugglec/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/mugglec/actions/workflows/win-msvc.yaml/badge.svg?branch=master)
![macos-clang](https://github.com/MuggleWei/mugglec/actions/workflows/macos-clang.yaml/badge.svg?branch=master)
![android](https://github.com/MuggleWei/mugglec/actions/workflows/android-cmake.yaml/badge.svg?branch=master)

<p align="center"><b>A Cross Platform C Base Library</b></p>
<p align="center">English | <a href="./README_cn.md">中文</a></p>

mugglec is a cross platform C base library, contains utilities like basic data structure, network, concurrency, log, crypt, memory pool, path operation and so on.

### Table of content
- [mugglec](#mugglec)
  - [Table of content](#table-of-content)
  - [Feature](#feature)
  - [Build](#build)
  - [tutorial and examples](#tutorial-and-examples)
  - [doc](#doc)

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
| BUILD_TESTING | OFF | build unittests |
| MUGGLE_BUILD_TRACE | OFF | build with trace info in debug |
| MUGGLE_BUILD_SANITIZER | OFF | build with sanitizer |
| MUGGLE_BUILD_COVERAGE | OFF | build with coverage(support with gcc and clang) |
| MUGGLE_BUILD_EXAMPLE | OFF | build examples |
| MUGGLE_BUILD_STRESS_TEST | OFF | build stress testing |
| MUGGLE_BUILD_BENCHMARK_UTILS | OFF | build benchmark utils |
| MUGGLE_BUILD_BENCHMARK | OFF | build benchmark |
| MUGGLE_INSTALL_BIN | OFF | install example, unittest and benchmark binaries |

NOTE: mugglec unittest use gtest, so if BUILD_TESTING is ON, it will try find gtest first, if gtest not found, will download gtest in first time automaticlly.  

### tutorial and examples
See [examples](./examples/readme_cn.md) to find tutorial and examples.  
If cmake build with option `MUGGLE_BUILD_EXAMPLE` ON, then [examples](./examples/readme_cn.md) folder will be build automatically.  

### doc
There are `Doxyfile` in the project root directory, user can install [doxygen](https://doxygen.nl/), and run `gen_doxygen_doc.sh` that in the project root directory to generate project doc.   
