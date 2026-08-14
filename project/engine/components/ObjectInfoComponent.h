#pragma once
#include "EngineDefines.h"
#include "design-patterns/component/EntityUuid.h"

namespace QFE::SCENE {
	/// @brief オブジェクトの情報を表すコンポーネントです。
	struct ObjectInfoComponent {
		std::string uuid = QFE::GenerateEntityUuid(); ///< 保存後も変わらないEntityのUUID
		std::string name; ///< オブジェクトの名前
		QFE_REFLECT_BEGIN(ObjectInfoComponent)
			QFE_REFLECT_MEMBER(uuid)
			QFE_REFLECT_MEMBER(name)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(ObjectInfoComponent)
}
