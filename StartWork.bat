@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -STA -File "%~dp0project\tools\StartWork.ps1"
if errorlevel 1 pause
