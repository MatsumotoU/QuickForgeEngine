@echo off
cd /d "%~dp0"
echo Pushing local tags to remote repository...
git push origin --tags
echo Done.
pause