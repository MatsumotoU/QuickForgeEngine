#pragma once
#include "EngineDefines.h"
#include "ecs/component/EntityUuid.h"
#include <vector>

namespace QFE::SCENE {
	/// @brief オブジェクトの情報を表すコンポーネントです。
	struct ObjectInfoComponent {
		std::string uuid = QFE::GenerateEntityUuid(); ///< 保存後も変わらないEntityのUUID
		std::string name; ///< オブジェクトの名前
		std::string hierarchyGroup; ///< エディターのヒエラルキー表示用グループ名
		std::vector<std::string> tags{ "Default" }; ///< 未設定の旧シーンも Default として扱う。
		QFE_REFLECT_BEGIN(ObjectInfoComponent)
			QFE_REFLECT_MEMBER(uuid)
			QFE_REFLECT_MEMBER(name)
			QFE_REFLECT_MEMBER(hierarchyGroup)
			QFE_REFLECT_MEMBER(tags)
		QFE_REFLECT_END()
	};
}
