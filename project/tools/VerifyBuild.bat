@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "SCRIPT_PATH=%SCRIPT_DIR%VerifyBuild.ps1"

echo QuickForgeEngine build verification
echo.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_PATH%" %*
set "EXIT_CODE=%ERRORLEVEL%"

echo.
if "%EXIT_CODE%"=="0" (
    echo Verification completed successfully.
) else (
    echo Verification failed with exit code %EXIT_CODE%.
)

echo.
pause
exit /b %EXIT_CODE%
