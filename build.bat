@echo off
REM Fast Batch Image Utility - Build Script
REM This script builds the project using CMake

setlocal enabledelayedexpansion

REM Get the directory where this script is located
set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build
set QT_PATH=E:/dev_tool/Qt/6.8.1/msvc2022_64

REM Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Change to build directory
cd /d "%BUILD_DIR%"

REM Run CMake
echo Running CMake configuration...
cmake "%SCRIPT_DIR%." -DCMAKE_PREFIX_PATH="%QT_PATH%"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed!
    echo.
    echo If Qt6 is not found, try specifying the path:
    echo   cmake "%SCRIPT_DIR%" -DCMAKE_PREFIX_PATH="%QT_PATH%"
    pause
    exit /b %ERRORLEVEL%
)

REM Build the project
echo.
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed!
    pause
    exit /b %ERRORLEVEL%
)

REM Deploy Qt dependencies
echo.
echo Deploying Qt dependencies...
"%QT_PATH%\bin\windeployqt.exe" --release "%BUILD_DIR%\bin\Release\fbiu_gui.exe"

echo.
echo Build completed successfully!
echo Executables are in: %BUILD_DIR%\bin\Release\
echo.
pause
