#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace FileLoader {
	/// <summary>
	/// ファイルを文字列として読み込む関数
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns></returns>
	std::string ReadFile(const std::string& filePath);
	/// <summary>
	/// 文字列を指定した区切り文字で分割する関数
	/// </summary>
	/// <param name="str">文字列</param>
	/// <param name="delimiter">区切り文字</param>
	/// <returns></returns>
	std::vector<std::string> Split(const std::string& str, char delimiter);
}