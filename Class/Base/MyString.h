#pragma once
#include <Windows.h>
#include <string>
#include <format>

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
