@echo off
setlocal enabledelayedexpansion

echo Generating BuildInfo.json...

:: Git情報の取得
set GIT_COMMIT=unknown
set GIT_BRANCH=unknown
for /f "tokens=*" %%i in ('git rev-parse --short HEAD 2^>nul') do set GIT_COMMIT=%%i
for /f "tokens=*" %%i in ('git rev-parse --abbrev-ref HEAD 2^>nul') do set GIT_BRANCH=%%i

:: 日時の取得（%TIME% の先頭スペースを0に置換）
set CURRENT_DATE=%DATE%
set CURRENT_TIME=%TIME: =0%

:: 出力先をバッチファイル位置から相対指定（
set "OUTFILE=%~dp0BuildInfo.json"

:: ファイル書き出し（ブロックリダイレクトで上書き）
(
    echo {
    echo     "commit": "%GIT_COMMIT%",
    echo     "branch": "%GIT_BRANCH%",
    echo     "date": "%CURRENT_DATE%",
    echo     "time": "%CURRENT_TIME%"
    echo }
) > "%OUTFILE%"

:WAIT_FILE
if not exist "%OUTFILE%" (
    timeout /t 1 /nobreak > nul
    goto WAIT_FILE
)
exit /b 0