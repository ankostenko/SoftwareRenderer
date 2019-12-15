@echo off

IF NOT EXIST build mkdir build
pushd build

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2015\Community\VC\Auxiliary\Build\vcvars64.bat"
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
	call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
)

cl ../SoftwareRenderer/src/sr.cpp -O2 /link user32.lib gdi32.lib -incremental:no /OUT:asteroids.exe

del sr.obj
echo Build has completed!

popd