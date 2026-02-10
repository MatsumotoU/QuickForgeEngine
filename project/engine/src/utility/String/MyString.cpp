/**
 * @file MyString.cpp
 * @brief 文字列変換(string <-> wstring)などのユーティリティ関数群の実装
 */

#include "engine/include/utility/String/MyString.h"
#include <winrt/base.h>

namespace QFE {
	void Log(const std::string& message) {
		OutputDebugStringA(message.c_str());
	}

	std::wstring ConvertString(const std::string& str) {
		if (str.empty()) {
			return std::wstring();
		}

		auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
		if (sizeNeeded == 0) {
			return std::wstring();
		}
		std::wstring result(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
		return result;
	}

	std::string ConvertString(const std::wstring& str) {
		if (str.empty()) {
			return std::string();
		}

		auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
		if (sizeNeeded == 0) {
			return std::string();
		}
		std::string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
		return result;
	}

	std::vector<char> StringToCharVector(const std::string& str)
	{
		std::vector<char> charVector(str.begin(), str.end());
		charVector.push_back('\0'); // ヌル終端を追加
		return charVector;
	}

	bool IsUnorderedPairEqual(const std::pair<std::string, std::string>& p1, const std::pair<std::string, std::string>& p2) {
		return (p1.first == p2.first && p1.second == p2.second) ||
			(p1.first == p2.second && p1.second == p2.first);
	}

	std::wstring Utf8ToUtf16(const std::string& utf8) {
		if (utf8.empty()) return std::wstring();
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}

}
