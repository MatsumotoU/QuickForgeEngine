#pragma once
#include <Windows.h>
#include <cstdint>
#include "Data/WindowConfigData.h"

namespace QFE {

	class WindowEventsManager;

	class WindowGenerater {
	public:
		static void CreateGameWindow(WNDCLASS& wc, HWND& hwnd,WindowConfigData& config, WNDPROC& proc,WindowEventsManager* eventManager);
	};

}
