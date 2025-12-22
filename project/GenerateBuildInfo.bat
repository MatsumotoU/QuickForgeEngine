@echo off
setlocal enabledelayedexpansion

:: デフォルト値（失敗した時用）を設定
set GIT_COMMIT=unknown
set GIT_BRANCH=unknown

:: Gitが使えるか、リポジトリかを確認しつつ取得
:: 2>nul をつけることでエラーメッセージを非表示にします
for /f "tokens=*" %%i in ('git rev-parse --short HEAD 2^>nul') do set GIT_COMMIT=%%i
for /f "tokens=*" %%i in ('git rev-parse --abbrev-ref HEAD 2^>nul') do set GIT_BRANCH=%%i

:: ファイル書き出し
echo #pragma once > BuildInfo.h
echo #define BUILD_COMMIT "%GIT_COMMIT%" >> BuildInfo.h
echo #define BUILD_BRANCH "%GIT_BRANCH%" >> BuildInfo.h

:: 何があっても「成功(0)」を返してビルドを続行させる
exit /b 0