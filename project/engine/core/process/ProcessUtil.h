#pragma once
#include <cstdint>
#include <string>

namespace QFE {
    namespace ProcessUtil {
		/// @brief 指定されたexeファイルを引数付きで起動する関数
        bool LaunchExe(const std::string& exePath, const std::string& arguments);

		/// @brief 外部プロセスを起動して終了まで待機し、標準出力と標準エラーをファイルへ保存する。
		bool RunExeAndWait(
			const std::wstring& exePath,
			const std::wstring& arguments,
			const std::wstring& workingDirectory,
			const std::wstring& outputPath,
			uint32_t& exitCode);
    }
}
