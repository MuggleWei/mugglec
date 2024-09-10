@echo off

setlocal ENABLEDELAYEDEXPANSION

:: handle argv
set count=0
for %%x in (%*) do (
    set /a count+=1
)

if %count% LSS 2 (
    echo "[ERROR] build.bat without build type"
	echo "[ERROR] Usage: build.bat <Debug|Release> <shared|static>"
    exit /b
)

set BUILD_TYPE=%~1
set LIB_TYPE=%~2

set "CMAKE_SHARED_LIBS=ON"
if "%LIB_TYPE%"=="shared" (
	set "CMAKE_SHARED_LIBS=ON"
) else (
	set "CMAKE_SHARED_LIBS=OFF"
)

echo build type: %BUILD_TYPE%
echo lib type: %LIB_TYPE%
echo CMAKE_SHARED_LIBS: %CMAKE_SHARED_LIBS%

set origin_dir=%~dp0
set build_dir=%origin_dir%build
set install_dir=%origin_dir%dist
set dep_search_path=f:\\.local\\usr

if exist %build_dir% (
	echo remove build dir
	rd /s /q %build_dir%
)
echo make build dir
md %build_dir%

if exist %install_dir% (
	echo remove install dir
	rd /s /q %install_dir%
)

cd %build_dir%

cmake ^
	-S %origin_dir% -B %build_dir% ^
	-DCMAKE_PREFIX_PATH=%dep_search_path% ^
	-DBUILD_SHARED_LIBS=%CMAKE_SHARED_LIBS% ^
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
