#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <Windows.h>
#include <format>
#include <utility>

namespace QFE {

	/// <summary>
	/// 出力ウィンドウにログをEす関数
	/// </summary>
	/// <param name="message">表示する斁EE</param>
	void Log(const std::string& message);
	/// <summary>
	/// std::stringからstd::wstringへ変換
	/// </summary>
	/// <param name="str">変換允E/param>
	/// <returns>変換允E/returns>
	std::wstring ConvertString(const std::string& str);
	/// <summary>
	/// wstd::stringからstd::stringへ変換
	/// </summary>
	/// <param name="str">変換允E/param>
	/// <returns>変換允E/returns>
	std::string ConvertString(const std::wstring& str);

	// stringからcharPtrに変換
	char* StringToCharPtr(const std::string& str);

	// E対の斁EEペアが頁E同で一致してぁEかどぁEを判断しまぁE
	bool IsUnorderedPairEqual(const std::pair<std::string, std::string>& p1, const std::pair<std::string, std::string>& p2);

	std::wstring Utf8ToUtf16(const std::string& utf8);

}
