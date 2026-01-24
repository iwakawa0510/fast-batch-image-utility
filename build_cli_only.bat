@echo off
REM Fast Batch Image Utility - CLI Only Build Script
REM This script builds only the CLI version (no Qt6 required)

setlocal enabledelayedexpansion

REM Get the directory where this script is located
set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build

REM Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Change to build directory
cd /d "%BUILD_DIR%"

REM Run CMake with GUI disabled
echo Running CMake configuration (CLI only)...
cmake "%SCRIPT_DIR%" -DBUILD_GUI=OFF

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

REM Build the project
echo.
echo Building CLI application...
cmake --build . --config Release --target fbiu_cli

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Build completed successfully!
echo Executable is in: %BUILD_DIR%\bin\Release\fbiu_cli.exe
echo.
pause
