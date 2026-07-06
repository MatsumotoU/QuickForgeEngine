#pragma once
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief オブジェクトの情報を表すコンポーネントです。
	struct ObjectInfoComponent {
		std::string name; ///< オブジェクトの名前
		QFE_REFLECT_BEGIN(ObjectInfoComponent)
			QFE_REFLECT_MEMBER(name)
		QFE_REFLECT_END()
	};
}