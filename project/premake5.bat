@echo off
setlocal

set PREMAKE_EXE=premake5.exe
set TARGET_IDE=vs2022

echo ====================================
echo Premake Build System Generator
echo Target: %TARGET_IDE%
echo ====================================

:: ------------------------------------
:: 0. 【ファイルのロックチェック】
:: ------------------------------------
echo Checking for locked project/solution files...

:: チェック対象の拡張子を定義
set "CHECK_EXTS=*.sln *.vcxproj"

for /r %%f in (%CHECK_EXTS%) do (
    if exist "%%f" (
        ren "%%f" "%%f" 2>nul
        if errorlevel 1 (
            echo.
            echo ---------------------------------------------------------
            echo Error: [%%f] 
            echo The file is currently in use by Visual Studio or another process.
            echo Please close the solution and try again.
            echo ---------------------------------------------------------
            pause
            exit /b 1
        )
    )
)

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
pause
endlocal
exit /b 0