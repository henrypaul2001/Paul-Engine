@echo off

echo Generating Projects...
call vendor\binaries\premake\premake5.exe vs2022
popd
PAUSE