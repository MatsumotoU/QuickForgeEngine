#pragma once
#include <string>
#include <stdint.h>

namespace QFE {
	/// @brief DirectInputで使用されるキーコードを文字列に変換します。
	std::string DirectInputToString(uint32_t keyCode);
} 