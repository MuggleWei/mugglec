## muggleCC
****
muggleCC is a pure C basic library written for myself, contains utilities like concurrency, memory pool, network, path operation, logging, crypt and so on. I keep it as tiny as possible, easy to use and cross-platform, so far, it work well on the systems I use often, recently linux version (with gcc 5.4, 7.4), win10.  

### How to build
It's easy to build this project with CMake, contains muggleCC library, example, unittest and benchmark, if you just wanna use library, can close other corresponding build options. 
About unittest, I use gtest, if switch BUILD_TESTING options on, will download gtest in first time of CMake build, this will takes some time, If your internet speed is not good, remember switch BUILD_TESTING and DMUGGLE_BUILD_TESTING off.  
If you just wanna use this library and learn how to use it with examples, just build library and example:  
```
cmake .. \
    -DMUGGLE_BUILD_TRACE=OFF \
    -DMUGGLE_BUILD_SHARED_LIB=ON \
    -DMUGGLE_BUILD_EXAMPLE=ON \
    -DMUGGLE_BUILD_TESTING=OFF \
    -DMUGGLE_BUILD_BENCHMARK=OFF \
    -DBUILD_TESTING=OFF
```
* MUGGLE_BUILD_TRACE: just for debug, you will never switch it on   
* MUGGLE_BUILD_SHARED_LIB: build shared lib or static lib   
* MUGGLE_BUILD_EXAMPLE: build muggleCC examples   
* MUGGLE_BUILD_TESTING: build muggleCC testings, if it's on, BUILD_TESTING will be set ON   
* MUGGLE_BUILD_BENCHMARK: for performance test   
* BUILD_TESTING: build gtest, if it's on, will download gtest in first time of cmake build, if you need gtest, don't download repeatedly   
