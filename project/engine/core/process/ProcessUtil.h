#pragma once
#include <cstdint>
#include <string>

namespace QFE {
    namespace ProcessUtil {
		/// @brief 指定されたexeファイルを引数付きで起動する関数
        bool LaunchExe(const std::string& exePath, const std::string& arguments);

		/// @brief 外部プロセスをコンソールウィンドウで起動し、終了まで待機する。
		bool RunExeAndWait(
			const std::wstring& exePath,
			const std::wstring& arguments,
			const std::wstring& workingDirectory,
			uint32_t& exitCode);
    }
}
