#pragma once
#include <string>

class CheckFileExtension final{
public:
	/// <summary>
	/// 指定のファイル名が指定の拡張子を持っているかどうかを確認します。
	/// </summary>
	/// <param name="fileName"></param>
	/// <param name="extension"></param>
	/// <returns></returns>
	[[nodiscard]] static bool HasExtension(const std::string& fileName, const std::string& extension);
};