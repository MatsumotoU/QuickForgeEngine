#include "HwndConvertString.h"

std::wstring HwndConvertString::HwndToString(HWND hwnd) {
	wchar_t buffer[256];
	GetWindowText(hwnd, buffer, sizeof(buffer) / sizeof(wchar_t));
	return std::wstring(buffer);
}