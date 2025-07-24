#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <windows.h>
#include <commdlg.h>

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
	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	std::string OpenFileDialog();
	/// <summary>
	/// 指定ディレクトリ内の特定の拡張子を持つファイル名一覧を取得する関数
	/// </summary>
	/// <param name="directoryPath">ディレクトリのパス</param>
	/// <param name="extension">拡張子（例: ".txt"）</param>
	/// <returns>ファイル名のリスト</returns>
	std::vector<std::string> GetFilesWithExtension(const std::string& directoryPath, const std::string& extension);
	/// <summary>
	/// フルパスからファイル名を抽出する関数(拡張子抜き)
	/// </summary>
	/// <param name="fullPath"></param>
	/// <returns></returns>
	std::string ExtractFileName(const std::string& fullPath);
}