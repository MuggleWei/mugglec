@echo off

setlocal ENABLEDELAYEDEXPANSION

set origin_dir=%~dp0
set user_local_dir=f:\\.local
set build_dir=%origin_dir%build
set install_dir=%user_local_dir%

md %build_dir%
cd %build_dir%

cmake .. ^
	-S %origin_dir% -B %build_dir% ^
	-DMUGGLE_BUILD_SHARED_LIB=ON ^
	-DMUGGLE_BUILD_EXAMPLE=OFF ^
	-DMUGGLE_BUILD_TESTING=OFF ^
	-DMUGGLE_BUILD_BENCHMARK=OFF ^
	-DMUGGLE_BUILD_TRACE=OFF ^
	-DMUGGLE_EXTRA_PREFIX_PATH=%user_local_dir% ^
	-DMUGGLE_INSTALL_BIN=OFF ^
	-DCMAKE_INSTALL_PREFIX=%install_dir% ^
	-G "Visual Studio 16 2019" -A x64

:: cmake --build . --config Release --target INSTALL
cmake --build . --config Release
