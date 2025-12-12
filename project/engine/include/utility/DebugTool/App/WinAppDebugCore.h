#pragma once
#include "CrashHandler.h"

class WinAppDebugCore final {
public:
	WinAppDebugCore(const LPSTR& lpCmdLine);
	~WinAppDebugCore();
};