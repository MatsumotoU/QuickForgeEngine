@echo off
setlocal EnableExtensions EnableDelayedExpansion

if "%~1"=="" (
    echo Usage: BuildProject.bat project-file [MSBuild options]
    exit /b 1
)

set "MSBUILD="
if defined MSBUILD_EXE_PATH if exist "%MSBUILD_EXE_PATH%" set "MSBUILD=%MSBUILD_EXE_PATH%"
if not defined MSBUILD if defined VSINSTALLDIR if exist "%VSINSTALLDIR%\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=%VSINSTALLDIR%\MSBuild\Current\Bin\MSBuild.exe"
if defined MSBUILD goto found_msbuild
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" for /f "usebackq delims=" %%I in (`""!VSWHERE!" -latest -products * -requires Microsoft.Component.MSBuild Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "MSBuild\**\Bin\MSBuild.exe""`) do if not defined MSBUILD set "MSBUILD=%%I"
if not defined MSBUILD for /f "delims=" %%I in ('where MSBuild.exe 2^>nul') do if not defined MSBUILD set "MSBUILD=%%I"
:found_msbuild
if not defined MSBUILD (
    echo MSBuild was not found. Install Visual Studio with the C++ build tools or set MSBUILD_EXE_PATH.
    exit /b 1
)

setlocal DisableDelayedExpansion
echo Building project using "%MSBUILD%"
"%MSBUILD%" %*
exit /b %ERRORLEVEL%
