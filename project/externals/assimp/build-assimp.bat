@echo off
setlocal EnableExtensions DisableDelayedExpansion

set "ASSIMP_VERSION=5.3.0"
set "ASSIMP_ROOT=%~dp0"
set "ASSIMP_SOURCE=%ASSIMP_ROOT%source"
set "ASSIMP_BUILD=%ASSIMP_ROOT%..\..\..\generated\assimp"
set "REQUESTED_CONFIGURATION=%~1"

if not defined REQUESTED_CONFIGURATION set "REQUESTED_CONFIGURATION=Debug"
if /i "%REQUESTED_CONFIGURATION%"=="Development" (
    set "CMAKE_CONFIGURATION=Release"
) else if /i "%REQUESTED_CONFIGURATION%"=="Release" (
    set "CMAKE_CONFIGURATION=Release"
) else (
    set "CMAKE_CONFIGURATION=Debug"
)

call :find_cmake
if errorlevel 1 exit /b 1

call :ensure_source
if errorlevel 1 exit /b 1

echo Configuring Assimp %ASSIMP_VERSION%...
"%CMAKE_EXE%" -Wno-dev -S "%ASSIMP_SOURCE%" -B "%ASSIMP_BUILD%" -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_ZLIB=ON -DASSIMP_WARNINGS_AS_ERRORS=OFF -DASSIMP_INSTALL=OFF -DUSE_STATIC_CRT=ON -DASSIMP_IGNORE_GIT_HASH=ON
if errorlevel 1 (
    echo Error: Failed to configure Assimp.
    exit /b 1
)

echo Building Assimp %ASSIMP_VERSION% ^(%CMAKE_CONFIGURATION% x64^)...
"%CMAKE_EXE%" --build "%ASSIMP_BUILD%" --config "%CMAKE_CONFIGURATION%" --target assimp --parallel
if errorlevel 1 (
    echo Error: Failed to build Assimp.
    exit /b 1
)

echo Assimp build complete.
exit /b 0

:find_cmake
set "CMAKE_EXE="
for /f "delims=" %%C in ('where cmake.exe 2^>nul') do if not defined CMAKE_EXE set "CMAKE_EXE=%%C"
if defined CMAKE_EXE exit /b 0

set "VS_CMAKE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if exist "%VS_CMAKE%" (
    set "CMAKE_EXE=%VS_CMAKE%"
    exit /b 0
)

echo Error: CMake was not found.
exit /b 1

:ensure_source
if exist "%ASSIMP_SOURCE%\CMakeLists.txt" exit /b 0
if exist "%ASSIMP_SOURCE%\" (
    echo Error: "%ASSIMP_SOURCE%" exists but is incomplete.
    exit /b 1
)

echo Downloading Assimp %ASSIMP_VERSION% source...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference='Stop'; $root='%ASSIMP_ROOT%'; $version='%ASSIMP_VERSION%'; $source=Join-Path $root 'source'; $zip=Join-Path ([IO.Path]::GetTempPath()) ('assimp-' + [Guid]::NewGuid().ToString() + '.zip'); $temp=Join-Path ([IO.Path]::GetTempPath()) ('assimp-' + [Guid]::NewGuid().ToString()); try { Invoke-WebRequest -Uri ('https://github.com/assimp/assimp/archive/refs/tags/v' + $version + '.zip') -OutFile $zip; Expand-Archive -LiteralPath $zip -DestinationPath $temp -Force; $expanded=Get-ChildItem -LiteralPath $temp -Directory | Select-Object -First 1; if ($null -eq $expanded -or -not (Test-Path -LiteralPath (Join-Path $expanded.FullName 'CMakeLists.txt'))) { throw 'The downloaded Assimp archive is invalid.' }; Move-Item -LiteralPath $expanded.FullName -Destination $source; } finally { if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force -ErrorAction SilentlyContinue }; if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Recurse -Force -ErrorAction SilentlyContinue } }"
if errorlevel 1 (
    echo Error: Failed to download Assimp %ASSIMP_VERSION%.
    exit /b 1
)
if not exist "%ASSIMP_SOURCE%\CMakeLists.txt" (
    echo Error: Assimp source was not installed correctly.
    exit /b 1
)
exit /b 0
