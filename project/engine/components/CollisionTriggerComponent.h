#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief コライダーのEnter・Stay・Exitから汎用パラメータ操作を発行する。
	struct CollisionTriggerComponent {
		uint32_t acceptedMask = 0; ///< 0なら全マスクを受け付ける。
		std::vector<std::string> acceptedTags; ///< 空ならタグを制限せず、複数指定時はいずれかが一致すればよい。
		nlohmann::json actions = nlohmann::json::array();

		// ランタイム接触状態（シーンには保存しない）。
		std::unordered_map<uint32_t, uint32_t> previousContacts;
		std::unordered_map<uint32_t, uint32_t> currentContacts;

		QFE_REFLECT_BEGIN(CollisionTriggerComponent)
			QFE_REFLECT_MEMBER(acceptedMask)
			QFE_REFLECT_MEMBER(acceptedTags)
			QFE_REFLECT_MEMBER(actions)
		QFE_REFLECT_END()
	};
}
