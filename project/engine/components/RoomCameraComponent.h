#pragma once

#include "EngineDefines.h"
#include "ecs/component/EntityReference.h"
#include <cstdint>

namespace QFE::SCENE {
	/// @brief 対象が入った部屋へカメラを移動する設定。
	struct RoomCameraComponent {
		QFE::EntityReference target;
		float moveSpeed = 4.0f; ///< 大きいほど目的位置に速く近づく。
		bool enabled = true;

		// 一時的にどの部屋にも入っていない間も移動先を保つ。シーンには保存しない。
		uint32_t currentRoom = UINT32_MAX;

		QFE_REFLECT_BEGIN(RoomCameraComponent)
			QFE_REFLECT_MEMBER(target)
			QFE_REFLECT_MEMBER(moveSpeed)
			QFE_REFLECT_MEMBER(enabled)
		QFE_REFLECT_END()
	};
}
