#pragma once
#include <string>
#include <stdint.h>
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief 球形コライダーコンポーネントの情報を保持する構造体
	struct SphereColliderComponent {
		float radius;
		uint32_t mask; // 衝突判定用のタグ
		
		std::string onCollisionEnterScriptFunctionName;

		QFE_REFLECT_BEGIN(SphereColliderComponent)
			QFE_REFLECT_MEMBER(radius)
			QFE_REFLECT_MEMBER(mask)
			QFE_REFLECT_MEMBER(onCollisionEnterScriptFunctionName)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SphereColliderComponent)
}