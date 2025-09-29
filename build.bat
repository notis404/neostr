@echo off

if not exist .\build (
	mkdir .\build
)

set "build_mode="
if /I [%~1]==[debug] set "build_mode=debug"
if /I [%~1]==[release] set "build_mode=release"

if not exist .\build\%build_mode% (
	mkdir .\build\%build_mode%
)
pushd build\%build_mode%
set "root=%~dp0"
set "windowsEscapedRoot=%root:\=\\%"

set "debugOptions=/MTd /Od /DDEBUG"
set "releaseOptions=/MT /O2 /GL /DRELEASE"
set "sharedOptions=/W4 /WX /TC /Zi /DPROJECT_ROOT=%windowsEscapedRoot%"

set "allOptions="
if /I [%build_mode%]==[debug] (
	set "allOptions=%debugOptions%"
)

if /I [%build_mode%]==[release] (
	set "allOptions=%releaseOptions%"
)
set "allOptions=%allOptions% %sharedOptions%"
cl %allOptions% %root%\src\main.c /link /DEBUG /SUBSYSTEM:CONSOLE /out:neo_string_example.exe
set "buildResult=%ERRORLEVEL%"
if /I [%build_mode%]==[debug] (
	if /I [%buildResult%]==[0] (
		call radbin %cd%\neo_string_example.pdb --out:%cd%\neo_string_example.rdi
	)
)
