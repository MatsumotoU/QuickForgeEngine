#pragma once
#include <string>

namespace QFE {
    namespace ProcessUtil {
		/// @brief 指定されたexeファイルを引数付きで起動する関数
        bool LaunchExe(const std::string& exePath, const std::string& arguments);
    }
}