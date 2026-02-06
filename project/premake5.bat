@echo off

setlocal



set PREMAKE_EXE=premake5.exe

set TARGET_IDE=vs2022

if not exist "%PREMAKE_EXE%" (
    echo [0/2] %PREMAKE_EXE% not found. Downloading...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $url = 'https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip'; $zip = 'premake.zip'; Invoke-WebRequest -Uri $url -OutFile $zip; $shell = New-Object -ComObject Shell.Application; $zipFile = $shell.NameSpace((Get-Item $zip).FullName); $dest = $shell.NameSpace((Get-Item .).FullName); $dest.CopyHere($zipFile.Items(), 16); Remove-Item $zip"
    if not exist "%PREMAKE_EXE%" (
        echo Error: Failed to download %PREMAKE_EXE%
        if not defined GITHUB_ACTIONS pause
        exit /b 1
    )
    echo Download Complete.
    del example.* 2>nul
    del luasocket.* 2>nul
)



echo ====================================

echo Premake Build System Generator

echo Target: %TARGET_IDE%

echo ====================================



:: ------------------------------------

:: 1. 【自動クリーンアップ処理の追加】

:: ------------------------------------

echo [1/2] Delete old files...



:: ソリューションファイル (.sln) の削除

del *.sln 2>nul

:: プロジェクトファイルの削除 (カレントディレクトリとそのサブディレクトリを検索 /s)

del /s /q *.vcxproj *.vcxproj.filters *.vcxproj.user 2>nul



echo Delete Complete.



:: ------------------------------------

:: 2. Premakeによるプロジェクトファイル生成

:: ------------------------------------

echo [2/2] Generate projects...



"%PREMAKE_EXE%" %TARGET_IDE%



if %errorlevel% neq 0 (
    echo.
    echo ------------------------------------
    echo Error:Faild generated file.
    echo ------------------------------------
    if not defined GITHUB_ACTIONS pause
    exit /b %errorlevel%
)



echo.

echo ------------------------------------

echo Generate complete.

echo %TARGET_IDE% 

echo ------------------------------------
if not defined GITHUB_ACTIONS pause

endlocal

exit /b 0