#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <format>
#include <utility>
#include <codecvt>
#include <locale>

#define NOMINMAX
#include <windows.h>

namespace QFE {
	/// @brief stringからwstringに変換
	std::wstring ConvertString(const std::string& str);
	/// @brief wstringからstringに変換
	std::string ConvertString(const std::wstring& str);
	/// @brief std::stringを書き換え可能な std::vector<char> に変換する
	std::vector<char> StringToCharVector(const std::string& str);
	/// @brief 順序を無視したペアの等価性を判定する
	bool IsUnorderedPairEqual(const std::pair<std::string, std::string>& p1, const std::pair<std::string, std::string>& p2);
	/// @brief UTF-16からUTF-8に変換
	std::wstring Utf8ToUtf16(const std::string& utf8);
}
