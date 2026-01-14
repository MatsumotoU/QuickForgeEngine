/**
 * @file MyString.cpp
 * @brief 文字列変換(string <-> wstring)などのユーティリティ関数群の実装
 */

#include "engine/include/utility/String/MyString.h"
#include <winrt/base.h>

/** @brief ログ出力関数 */
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

/** @brief stringからwstringに変換 */
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

/** @brief wstringからstringに変換 */
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

/** @brief stringをLPCWSTRに変換 */
LPCWSTR StringToLPCWSTR(const std::string& str) {
    LPCWSTR lpcwstr{};
    std::wstring wstr = ConvertString(str);
    lpcwstr = wstr.c_str();
    return lpcwstr;
}

/** @brief exeを指定してファイルを開く */
bool QFE::FILE::OpenFileOnExe(const std::string& exePath, const std::string& filePath) {
    // ShellExecuteAの戻り値が32以下なら失敗
    HINSTANCE result = ShellExecuteA(
        NULL,           // ウィンドウハンドル
        "open",         // 操作
        exePath.c_str(),// 実行するexe
        filePath.c_str(),// 引数(ここでは開きたいファイルパス)
        NULL,           // カレントディレクトリ
        SW_SHOWNORMAL   // ウィンドウ表示方法
    );
    return reinterpret_cast<intptr_t>(result) > 32;
}

/** @brief JSONをファイルに保存 */
bool QFE::FILE::SaveJSONToFile(const std::string& filePath, const nlohmann::json& json) {
	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		ofs << json.dump(4); // インデント幅4で整形して保存
		ofs.close();
		return true;
	}
    return false;
}

/**
 * @brief std::stringを書き換え可能な char* に変換する
 * TODO: 呼び出し側で delete[] buffer が必須。スマートポインタや std::vector<char> への移行を検討。
 */
char* StringToCharPtr(const std::string& str) {
    if (str.empty()) {
        return nullptr;
    }
    // ヌル終端を含めたバッファを確保
    char* buffer = new char[str.size() + 1];
    std::memcpy(buffer, str.c_str(), str.size() + 1); // ヌル終端もコピー
    return buffer;
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
