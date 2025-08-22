#pragma once
#include "CrashHandler.h"

// MainCommand
/// /DISABLE_D3D12_DEBUG_WARNING (グラボの警告を無視する)
/// /DISABLE_D3D12_DEBUG_ERROR (グラボのエラーを無視する)

class WinAppDebugCore final {
public:
	WinAppDebugCore(const LPSTR& lpCmdLine);
	~WinAppDebugCore();

private:


};