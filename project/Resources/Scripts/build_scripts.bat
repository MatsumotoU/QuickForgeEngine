@echo off
chcp 65001 > nul

echo Checking build arguments...

REM Check if project file argument is provided
if "%~1"=="" (
    echo Error: No project file specified.
    echo Usage: build_scripts.bat ^<ProjectFile.csproj^>
    pause
    exit /b 1
)

set PROJECT_PATH=%~1

REM Check if the project file exists
if not exist "%PROJECT_PATH%" (
    echo Error: Project file not found: %PROJECT_PATH%
    pause
    exit /b 1
)

echo Compiling C# project: %PROJECT_PATH%...

REM Navigate to the directory of this batch file
cd /d "%~dp0"

REM Define the log file path
set LOG_FILE=build_log.txt

REM Build the C# project and redirect all output to the log file.
echo --- Build started at %date% %time% --- > %LOG_FILE%
echo Project: %PROJECT_PATH% >> %LOG_FILE%
dotnet build "%PROJECT_PATH%" >> %LOG_FILE% 2>&1
set BUILD_ERROR_LEVEL=%errorlevel%

echo.
echo --- Build Log ---
type %LOG_FILE%
echo --- End of Log ---
echo.

REM Check the saved exit code to determine if the build was successful.
if %BUILD_ERROR_LEVEL% neq 0 (
    echo Build FAILED.
    pause
    exit /b %BUILD_ERROR_LEVEL%
) else (
    echo Build SUCCEEDED.
    echo Cleaning up obj folder...
    rd /s /q obj
    echo Cleanup complete.
)

exit /b 0