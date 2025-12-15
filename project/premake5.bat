@echo off
setlocal

set PREMAKE_EXE=premake5.exe
set TARGET_IDE=vs2022

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
    pause
    exit /b %errorlevel%
)

echo.
echo ------------------------------------
echo Generate complete.
echo %TARGET_IDE% 
echo ------------------------------------

endlocal
exit /b 0