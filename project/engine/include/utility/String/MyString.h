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
/// 蜃ｺ蜉帙え繧｣繝ｳ繝峨え縺ｫ繝ｭ繧ｰ繧貞・縺咎未謨ｰ
/// </summary>
/// <param name="message">陦ｨ遉ｺ縺吶ｋ譁・ｭ怜・</param>
void Log(const std::string& message);
/// <summary>
/// std::string縺九ｉstd::wstring縺ｸ螟画鋤
/// </summary>
/// <param name="str">螟画鋤蜈・/param>
/// <returns>螟画鋤蜈・/returns>
std::wstring ConvertString(const std::string& str);
/// <summary>
/// wstd::string縺九ｉstd::string縺ｸ螟画鋤
/// </summary>
/// <param name="str">螟画鋤蜈・/param>
/// <returns>螟画鋤蜈・/returns>
std::string ConvertString(const std::wstring& str);
/// <summary>
/// std::string縺九ｉLPCWSTR縺ｸ螟画鋤
/// </summary>
/// <param name="str">螟画鋤蜈・/param>
/// <returns>螟画鋤蜈・/returns>
LPCWSTR StringToLPCWSTR(const std::string& str);

// string縺九ｉcharPtr縺ｫ螟画鋤
char* StringToCharPtr(const std::string& str);

// ・貞ｯｾ縺ｮ譁・ｭ怜・繝壹い縺碁・ｸ榊酔縺ｧ荳閾ｴ縺励※縺・ｋ縺九←縺・°繧貞愛譁ｭ縺励∪縺・
bool IsUnorderedPairEqual(const std::pair<std::string, std::string>& p1, const std::pair<std::string, std::string>& p2);

std::wstring Utf8ToUtf16(const std::string& utf8);
