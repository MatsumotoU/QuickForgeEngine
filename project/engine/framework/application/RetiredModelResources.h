#pragma once
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace QFE::FRAMEWORK {
/// @brief 今フレーム使用しないEntity別バッファを所有一覧から外し、GPU完了待ち後の解放候補へ移す。
template<typename Handle>
std::vector<std::pair<uint32_t, Handle>> CollectUnusedAnimatedBuffers(
	std::unordered_map<uint32_t, Handle>& buffers, const std::unordered_set<uint32_t>& active) {
	std::vector<std::pair<uint32_t, Handle>> retired;
	for (auto it = buffers.begin(); it != buffers.end();) {
		if (active.contains(it->first)) { ++it; continue; }
		retired.emplace_back(it->first, it->second);
		it = buffers.erase(it);
	}
	return retired;
}
}
