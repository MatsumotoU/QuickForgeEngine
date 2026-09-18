@echo off
setlocal EnableExtensions DisableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%" >nul 2>&1
if errorlevel 1 (
    echo Error: Failed to enter the Premake directory.
    exit /b 1
)

set "PREMAKE_EXE=premake5.exe"
set "TARGET_IDE=vs2022"
set "REQUIRED_PREMAKE_VERSION=5.0.0-beta8"
set "PREMAKE_DOWNLOAD_URL=https://github.com/premake/premake-core/releases/download/v5.0.0-beta8/premake-5.0.0-beta8-windows.zip"
set "EXIT_CODE=0"

call :ensure_premake
if errorlevel 1 (
    set "EXIT_CODE=1"
    goto :finish
)

echo ====================================
echo Premake Build System Generator
echo Target: %TARGET_IDE%
echo Premake: %CURRENT_PREMAKE_VERSION%
echo ====================================

echo [1/2] Cleaning project files older than 1 day...

forfiles /s /m *.sln /d -1 /c "cmd /c echo Deleting: @path && del /f /q @path" 2>nul
forfiles /s /m *.vcxproj /d -1 /c "cmd /c echo Deleting: @path && del /f /q @path" 2>nul
forfiles /s /m *.vcxproj.filters /d -1 /c "cmd /c echo Deleting: @path && del /f /q @path" 2>nul
forfiles /s /m *.vcxproj.user /d -1 /c "cmd /c echo Deleting: @path && del /f /q @path" 2>nul

echo [2/2] Generating projects...

"%PREMAKE_EXE%" %TARGET_IDE%
set "EXIT_CODE=%errorlevel%"

if not "%EXIT_CODE%"=="0" (
    echo.
    echo ------------------------------------
    echo Error: Failed to generate project files.
    echo ------------------------------------
    if not defined GITHUB_ACTIONS pause
    goto :finish
)

echo.
echo ------------------------------------
echo Generation complete.
echo %TARGET_IDE%
echo ------------------------------------
if not defined GITHUB_ACTIONS pause
goto :finish

:ensure_premake
call :read_premake_version

if not defined CURRENT_PREMAKE_VERSION (
    echo Premake is missing or its version could not be read.
    goto :download_premake
)

call :compare_premake_version
if /i "%PREMAKE_VERSION_STATUS%"=="OK" (
    exit /b 0
)

echo Premake %CURRENT_PREMAKE_VERSION% is older than the required %REQUIRED_PREMAKE_VERSION%.
echo Updating Premake...

:download_premake
echo Downloading Premake %REQUIRED_PREMAKE_VERSION%...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference='Stop'; $root=(Get-Location).Path; $url='%PREMAKE_DOWNLOAD_URL%'; $zip=Join-Path $root 'premake5.download.zip'; $temp=Join-Path ([IO.Path]::GetTempPath()) ('premake-' + [Guid]::NewGuid().ToString()); try { New-Item -ItemType Directory -Path $temp -Force | Out-Null; Invoke-WebRequest -Uri $url -OutFile $zip; Expand-Archive -Path $zip -DestinationPath $temp -Force; $source=Get-ChildItem -Path $temp -Filter 'premake5.exe' -Recurse | Select-Object -First 1; if ($null -eq $source) { throw 'premake5.exe was not found in the downloaded archive.' }; Copy-Item -LiteralPath $source.FullName -Destination (Join-Path $root 'premake5.exe') -Force; } finally { if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force -ErrorAction SilentlyContinue }; if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Recurse -Force -ErrorAction SilentlyContinue } }"
if errorlevel 1 (
    echo Error: Failed to download or install Premake %REQUIRED_PREMAKE_VERSION%.
    exit /b 1
)

call :read_premake_version
call :compare_premake_version
if /i not "%PREMAKE_VERSION_STATUS%"=="OK" (
    echo Error: The installed Premake version is "%CURRENT_PREMAKE_VERSION%". Expected %REQUIRED_PREMAKE_VERSION% or newer.
    exit /b 1
)

echo Premake %CURRENT_PREMAKE_VERSION% is ready.
exit /b 0

:read_premake_version
set "CURRENT_PREMAKE_VERSION="

if not exist "%PREMAKE_EXE%" (
    exit /b 0
)

for /f "tokens=6" %%V in ('"%PREMAKE_EXE%" --version 2^>nul') do set "CURRENT_PREMAKE_VERSION=%%V"
exit /b 0

:compare_premake_version
set "PREMAKE_VERSION_STATUS="
for /f "delims=" %%V in ('powershell -NoProfile -ExecutionPolicy Bypass -Command "function Get-Key([string]$v) { if ($v -match '^([0-9]+)\.([0-9]+)\.([0-9]+)-beta([0-9]+)$') { return ([int64]$matches[1] * 1000000000000) + ([int64]$matches[2] * 1000000000) + ([int64]$matches[3] * 1000000) + [int64]$matches[4] }; if ($v -match '^([0-9]+)\.([0-9]+)\.([0-9]+)$') { return ([int64]$matches[1] * 1000000000000) + ([int64]$matches[2] * 1000000000) + ([int64]$matches[3] * 1000000) + 999999 }; return -1 }; if ((Get-Key '%CURRENT_PREMAKE_VERSION%') -ge (Get-Key '%REQUIRED_PREMAKE_VERSION%')) { 'OK' } else { 'OUTDATED' }"') do set "PREMAKE_VERSION_STATUS=%%V"
exit /b 0

:finish
popd
endlocal & exit /b %EXIT_CODE%
