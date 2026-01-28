@echo off
setlocal
pushd "%~dp0"

:: --- 設定項目 ---
:: チェックするディレクトリ（これらが存在すればインストール済みとみなす）
set "CHECK_DIR_BIN=bin"
set "CHECK_DIR_LIB=lib"

:: Monoバイナリが固められたZIPのURL
set "DOWNLOAD_URL=https://github.com/MatsumotoU/QuickForgeEngine/releases/download/mono2.0/mono-2.0.zip"

:: 一時保存用のファイル名
set "TEMP_ZIP=mono_temp.zip"

echo [QuickForgeEngine] Checking Mono Environment...

:: 1. 既に存在するか確認
if exist "%CHECK_DIR_BIN%" if exist "%CHECK_DIR_LIB%" (
    echo Mono binaries and libraries already exist. skipping setup.
    goto :SUCCESS
)

echo Mono environment incomplete. starting download...
echo From: %DOWNLOAD_URL%

:: 2. ダウンロード実行 (curlはWindows 10/11標準)
:: -L: リダイレクトを追う, -f: HTTPエラー時に失敗させる, -S: エラーを表示
curl -L -f -S -o "%TEMP_ZIP%" "%DOWNLOAD_URL%"

if %ERRORLEVEL% neq 0 (
    echo [Error] Failed to download Mono binaries.
    echo Please check the DOWNLOAD_URL in setup_mono.bat and ensure curl is accessible.
    if exist "%TEMP_ZIP%" del "%TEMP_ZIP%"
    popd
    exit /b 1
)

:: 3. 解凍実行 (PowerShellのExpand-Archiveを使用)
echo Extracting files...
powershell -Command "Expand-Archive -Path '%TEMP_ZIP%' -DestinationPath '.' -Force"

if %ERRORLEVEL% neq 0 (
    echo [Error] Failed to extract Mono binaries.
    if exist "%TEMP_ZIP%" del "%TEMP_ZIP%"
    popd
    exit /b 1
)

:: 4. 後片付け（一時ファイルを削除）
if exist "%TEMP_ZIP%" (
    echo Cleaning up temporary files...
    del "%TEMP_ZIP%"
)

echo Mono binaries installed successfully.

:SUCCESS
popd
endlocal
exit /b 0