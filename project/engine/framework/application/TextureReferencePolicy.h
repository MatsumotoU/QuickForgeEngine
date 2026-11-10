#pragma once
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

namespace QFE::FRAMEWORK {
/// @brief 現行ローダーがキューブマップとして扱うDDSかを判定する。
inline bool IsDdsTexture(const std::string& name) {
	const std::filesystem::path path(std::u8string(reinterpret_cast<const char8_t*>(name.data()), name.size()));
	std::string extension = path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return extension == ".dds";
}
}
