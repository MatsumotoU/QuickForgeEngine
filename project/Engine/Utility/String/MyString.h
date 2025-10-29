#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <Windows.h>
#include <format>
#include <utility>

/// <summary>
/// 出力ウィンドウにログを出す関数
/// </summary>
/// <param name="message">表示する文字列</param>
void Log(const std::string& message);
/// <summary>
/// std::stringからstd::wstringへ変換
/// </summary>
/// <param name="str">変換元</param>
/// <returns>変換先</returns>
std::wstring ConvertString(const std::string& str);
/// <summary>
/// wstd::stringからstd::stringへ変換
/// </summary>
/// <param name="str">変換元</param>
/// <returns>変換先</returns>
std::string ConvertString(const std::wstring& str);
/// <summary>
/// std::stringからLPCWSTRへ変換
/// </summary>
/// <param name="str">変換元</param>
/// <returns>変換先</returns>
LPCWSTR StringToLPCWSTR(const std::string& str);

// stringからcharPtrに変換
char* StringToCharPtr(const std::string& str);

// ２対の文字列ペアが順不同で一致しているかどうかを判断します
bool IsUnorderedPairEqual(const std::pair<std::string, std::string>& p1, const std::pair<std::string, std::string>& p2);

std::wstring Utf8ToUtf16(const std::string& utf8);