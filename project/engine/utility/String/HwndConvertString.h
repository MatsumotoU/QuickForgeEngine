#pragma once
#include <Windows.h>
#include <string>

namespace QFE {
	/// @brief HWNDと文字列の変換を行うクラス
	class HwndConvertString {
	public:
		// HWNDからstd::wstringに変換
		[[nodiscard]] static std::wstring HwndToString(HWND hwnd);
	};
}
