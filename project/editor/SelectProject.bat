@echo off
setlocal
if /i not "%~1"=="--hidden" (
    set "QFE_SELECT_PROJECT_BAT=%~f0"
    powershell.exe -NoProfile -WindowStyle Hidden -Command "Start-Process -FilePath $env:ComSpec -ArgumentList ('/d /c {0}{0}{1}{0} --hidden{0}' -f [char]34,$env:QFE_SELECT_PROJECT_BAT) -WindowStyle Hidden"
    exit /b
)
powershell.exe -NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -STA -File "%~dp0SelectProject.ps1"
exit /b %errorlevel%
