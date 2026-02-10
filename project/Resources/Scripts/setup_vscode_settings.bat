@echo off
setlocal
chcp 65001 > nul

REM Navigate to the directory where the batch file is located
cd /d "%~dp0"

set VSCODE_DIR=.vscode
set SETTINGS_FILE=%VSCODE_DIR%\settings.json

REM Check for /silent argument
set SILENT_MODE=0
if "%~1"=="/silent" set SILENT_MODE=1

echo VS Code環境の確認中...

REM settings.json が既に存在する場合は何もしない
if exist "%SETTINGS_FILE%" (
    echo %SETTINGS_FILE% は既に存在します。スキップします。
    goto :end
)

REM .vscode フォルダが存在しない場合は作成
if not exist "%VSCODE_DIR%" (
    mkdir "%VSCODE_DIR%"
    echo %VSCODE_DIR% フォルダを作成しました。
)

REM settings.json を生成
(
echo {
echo     "Lua.workspace.library": [
echo         "./Scripts"
echo     ]
echo }
) > "%SETTINGS_FILE%"

echo %SETTINGS_FILE% を作成しました。
echo セットアップが完了しました。

:end
if "%SILENT_MODE%"=="0" pause
exit /b 0
