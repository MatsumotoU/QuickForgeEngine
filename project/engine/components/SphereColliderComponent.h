#pragma once
#include <string>
#include <unordered_map>
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief 球形コライダーコンポーネントの情報を保持する構造体
	struct SphereColliderComponent {
		float radius;
		std::string tag;
		
		std::string onCollisionEnterScriptFunctionName;

		QFE_REFLECT_BEGIN(SphereColliderComponent)
			QFE_REFLECT_MEMBER(radius)
			QFE_REFLECT_MEMBER(tag)
			QFE_REFLECT_MEMBER(onCollisionEnterScriptFunctionName)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SphereColliderComponent)
}