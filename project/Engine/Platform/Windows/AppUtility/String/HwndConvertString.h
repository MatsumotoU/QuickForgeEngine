#pragma once
#include <Windows.h>
#include <string>

class HwndConvertString {
public:
	// HWNDからstd::wstringに変換
	[[nodiscard]] static std::wstring HwndToString(HWND hwnd);
};