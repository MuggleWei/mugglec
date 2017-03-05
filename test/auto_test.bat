@echo off
set build_dir=..\build
set debug_dir=%build_dir%\bin\Debug
set release_dir=%build_dir%\bin\Release
set debug_report_dir=%debug_dir%\UnitTest
set release_report_dir=%release_dir%\UnitTest

set "unit_tests=UnitTest_File UnitTest_Str unitTest_Atomic"

set report_debug="0"
set report_release="0"

if exist %build_dir% (
    if exist %debug_dir% (
		set report_debug="1"
	) else (
		set report_debug="0"
	)
	
	if exist %release_dir% (
		set report_release="1"
	) else (
		set report_release="0"
	)
)

for %%t in (%unit_tests%) do (
	if %report_debug% == "1" (
		%debug_dir%\%%t.exe --gtest_output=xml:%debug_report_dir%\%%t_report.xml
	)
	if %report_release% == "1" (
		%release_dir%\%%t.exe --gtest_output=xml:%release_report_dir%\%%t_report.xml
	)
)