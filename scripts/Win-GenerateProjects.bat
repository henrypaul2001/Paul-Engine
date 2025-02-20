@echo off
pushd ..\
call vendor\binaries\premake\premake5.exe vs2022
popd
PAUSE