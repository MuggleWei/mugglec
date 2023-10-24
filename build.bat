@echo off

setlocal ENABLEDELAYEDEXPANSION

set origin_dir=%~dp0
set build_dir=%origin_dir%build
set dep_search_path=f:\\.local\\usr
set install_dir=%origin_dir%dist

md %build_dir%
cd %build_dir%

cmake .. ^
	-S %origin_dir% -B %build_dir% ^
	-DCMAKE_PREFIX_PATH=%dep_search_path% ^
	-DBUILD_SHARED_LIBS=ON ^
	-DMUGGLE_BUILD_STATIC_PIC=ON ^
	-DMUGGLE_BUILD_TRACE=OFF ^
	-DMUGGLE_BUILD_SANITIZER=OFF ^
	-DBUILD_TESTING=OFF ^
	-DMUGGLE_BUILD_EXAMPLE=OFF ^
	-DMUGGLE_BUILD_BENCHMARK=OFF ^
	-DMUGGLE_INSTALL_BIN=OFF ^
	-DCMAKE_INSTALL_PREFIX=%install_dir%

:: cmake --build . --config Release --target INSTALL
cmake --build . --config Release
cmake --build . --config Release --target install
