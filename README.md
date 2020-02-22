## muggleCC
****
muggleCC is a C/C++ basic library written for myself, contains utilities like concurrency, memory pool, network, path operation, logging and so on. I keep it as tiny as possible, easy to use and cross-platform, so far, it work well on the systems I use often, recently linux version (with gcc 5.4, 7.4), win10.  

### How to build
It's easy to build this project with CMake, contains muggleCC library, example, unittest and benchmark, if you just wanna use library, can close other corresponding build options. In fact, most of codes in this library are pure C, you can build only part of pure C if you want.  
About unittest, I use gtest, if switch BUILD_TESTING options on, will download gtest in first time of CMake build, this will takes some time, If your internet speed is not good, remember switch BUILD_TESTING and DMUGGLE_BUILD_TESTING off.  
If you just wanna use this library and learn how to use it with examples, just build library and example:  
```
cmake .. \
    -DMUGGLE_BUILD_ONLY_C=OFF \
    -DMUGGLE_BUILD_TRACE=OFF \
    -DMUGGLE_BUILD_EXAMPLE=ON \
    -DMUGGLE_BUILD_TESTING=OFF \
    -DMUGGLE_BUILD_BENCHMARK=OFF \
    -DBUILD_TESTING=OFF
```