@echo off
REM Git����擾����BuildInfo.h�𐶐�
for /f %%i in ('git rev-parse --short HEAD') do set GIT_COMMIT=%%i
for /f %%i in ('git rev-parse --abbrev-ref HEAD') do set GIT_BRANCH=%%i

echo #pragma once > BuildInfo.h
echo #define BUILD_COMMIT "%GIT_COMMIT%" >> BuildInfo.h
echo #define BUILD_BRANCH "%GIT_BRANCH%" >> BuildInfo.h