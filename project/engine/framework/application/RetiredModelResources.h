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
/// @brief 現在の版が描画済みの登録モデルについて、旧版のキャッシュキーかを判定する。
inline bool IsRetiredModelResource(std::string_view key, const std::unordered_map<std::string, uint64_t>& currentRevisions) {
	constexpr std::string_view prefix = "asset-id/";
	if (!key.starts_with(prefix)) return false;
	key.remove_prefix(prefix.size());
	const auto idEnd = key.find('/');
	if (idEnd == std::string_view::npos || idEnd == 0) return false;
	const auto id = key.substr(0, idEnd);
	key.remove_prefix(idEnd + 1);
	const auto revisionEnd = key.find('/');
	if (revisionEnd == std::string_view::npos || revisionEnd == 0 || revisionEnd + 1 == key.size()) return false;
	uint64_t revision = 0;
	const auto result = std::from_chars(key.data(), key.data() + revisionEnd, revision);
	if (result.ec != std::errc{} || result.ptr != key.data() + revisionEnd || revision == 0) return false;
	const auto current = currentRevisions.find(std::string(id));
	return current != currentRevisions.end() && revision < current->second;
}

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
