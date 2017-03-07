# Muggle Lib


## Summary ##

This repo contain some basic codes for writing cross-platform C/C++ programs.
I divided all the codes into four parts: base c, utils c, base cpp, utils cpp.


## Build ##

You can easily build this repository with cmake, first enter the repository path, and do:
```
mkdir build
cd build
cmake ..
```

In above, it will build 4 seperate libs, if you wanna only one integral lib, just close 
BUILD_MUGGLE_BASE_SEPARATE option:
```
cmake .. -DBUILD_MUGGLE_BASE_SEPARATE=OFF
```

## Libs ##

base c (not finish yet): the most basic lib, include simple operation of log, 
c string, file,dll, memory pool, thread and sync etc.

utils c (not finish yet): some convenient c method, dependent on base c.

base cpp (not finish yet): the most basic cpp lib, resembel base c, contain 
most basic cpp code for easily write code.

utils cpp (not finish yet): some convenient cpp class, dependent on base cpp.