#pragma once

#include <cstdint>
#include <unordered_map>

#include "../../externals/nlohmann/json.hpp"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief コライダーのEnter・Stay・Exitから汎用パラメータ操作を発行する。
	struct CollisionTriggerComponent {
		uint32_t acceptedMask = 0; ///< 0なら全マスクを受け付ける。
		nlohmann::json actions = nlohmann::json::array();

		// ランタイム接触状態（シーンには保存しない）。
		std::unordered_map<uint32_t, uint32_t> previousContacts;
		std::unordered_map<uint32_t, uint32_t> currentContacts;

		QFE_REFLECT_BEGIN(CollisionTriggerComponent)
			QFE_REFLECT_MEMBER(acceptedMask)
			QFE_REFLECT_MEMBER(actions)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(CollisionTriggerComponent)
}
