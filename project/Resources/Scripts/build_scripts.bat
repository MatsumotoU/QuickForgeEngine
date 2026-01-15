
@echo off
chcp 65001 > nul

echo Compiling C# scripts...

REM Navigate to the directory of this batch file
cd /d "%~dp0"

REM Define the log file path
set LOG_FILE=build_log.txt

REM Build the C# project and redirect all output to the log file.
echo --- Build started at %date% %time% --- > %LOG_FILE%
dotnet build CSharpScripts.csproj >> %LOG_FILE% 2>&1
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
) else (
    echo Build SUCCEEDED.
    echo Cleaning up obj folder...
    rd /s /q obj
    echo Cleanup complete.
)

exit