@echo off
setlocal
rem Set CMAKE_PREFIX_PATH to your Qt 6 directory before running this script.
rem Use the Qt-compatible compiler's developer command prompt.
cmake -S "%~dp0" -B "%~dp0build" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
if errorlevel 1 exit /b 1
cmake --build "%~dp0build" --config Release
if errorlevel 1 exit /b 1
echo Build completed. Use windeployqt on the executable to bundle Qt runtime files.
