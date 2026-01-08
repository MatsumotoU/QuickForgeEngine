@echo off
setlocal enabledelayedexpansion

:: Git情報の取得
set GIT_COMMIT=unknown
set GIT_BRANCH=unknown
for /f "tokens=*" %%i in ('git rev-parse --short HEAD 2^>nul') do set GIT_COMMIT=%%i
for /f "tokens=*" %%i in ('git rev-parse --abbrev-ref HEAD 2^>nul') do set GIT_BRANCH=%%i

:: 日時の取得（%TIME% の先頭スペースを0に置換）
set CURRENT_DATE=%DATE%
set CURRENT_TIME=%TIME: =0%

:: ファイル書き出し
echo #pragma once > BuildInfo.h
echo #define BUILD_COMMIT "%GIT_COMMIT%" >> BuildInfo.h
echo #define BUILD_BRANCH "%GIT_BRANCH%" >> BuildInfo.h
echo #define BUILD_DATE "%CURRENT_DATE%" >> BuildInfo.h
echo #define BUILD_TIME "%CURRENT_TIME%" >> BuildInfo.h

:WAIT_FILE
if not exist "BuildInfo.h" (
    timeout /t 1 /nobreak > nul
    goto WAIT_FILE
)
exit /b 0