## mugglec
mugglec is a cross platform pure C base library, contains utilities like network, concurrency, memory pool, path operation, logging, crypt and so on. I keep it as tiny as possible and easy to use.  

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
| MUGGLE_BUILD_SHARED_LIB | ON | build shared library |
| MUGGLE_BUILD_STATIC_PIC | ON | if build static library, set position independent code flag |
| MUGGLE_BUILD_EXAMPLE | OFF | build examples |
| MUGGLE_BUILD_TESTING | OFF | build unittests |
| MUGGLE_BUILD_BENCHMARK | OFF | build benchmark |
| MUGGLE_BUILD_TRACE | OFF | build with trace info in debug |
| MUGGLE_EXTRA_PREFIX_PATH | "" | specify extra find path, it will append into CMAKE_PREFIX_PATH |

NOTE: mugglec unittest use gtest, so if MUGGLE_BUILD_TESTING is ON, it will try find gtest first, if gtest not found, will download gtest automaticlly.  
