#include "ProcessUtil.h"

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>

bool QFE::ProcessUtil::LaunchExe(const std::string& exePath, const std::string& arguments) {
    std::string commandLine = exePath + " " + arguments;
    std::vector<char> buffer(commandLine.begin(), commandLine.end());
    buffer.push_back('\0');

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(nullptr, buffer.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}
