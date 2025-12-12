#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

struct WindowConfigData {
	int32_t clientWidth = 800;  // width
	int32_t clientHeight = 600; // height
	std::wstring windowName = L"Game Window";

	bool isCanDropFiles = true;
};