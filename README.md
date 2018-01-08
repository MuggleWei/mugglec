# Muggle Lib


## Summary ##

This repo contain some basic codes for writing cross-platform C/C++ programs.


## Build ##

You can easily build this repository with cmake, first enter the repository path, and do:
```
mkdir build
cd build
cmake ..
```

In fact, this repo split two part, if you want, you can only build c component:
```
cmake .. -DMUGGLE_BUILD_ONLY_C=ON
```
