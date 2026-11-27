#pragma once

#include "EngineDefines.h"
#include <string>
#include <vector>
#include <algorithm>

namespace QFE::COMPONENTS {
	/// @brief ゲーム全体の入力を制御する。イベント等で inputEnabled を切り替える。
	struct InputControlComponent {
		bool inputEnabled = true;
		bool affectAllActions = true;
		std::vector<std::string> actions;

		// 複数の入力制御がある場合は、どれか一つが禁止すれば入力を受け付けない。
		[[nodiscard]] bool BlocksAction(const std::string& action) const {
			return !inputEnabled && (affectAllActions || std::find(actions.begin(), actions.end(), action) != actions.end());
		}

		QFE_REFLECT_BEGIN(InputControlComponent)
			QFE_REFLECT_MEMBER(inputEnabled)
			QFE_REFLECT_MEMBER(affectAllActions)
			QFE_REFLECT_MEMBER(actions)
		QFE_REFLECT_END()
	};
}
