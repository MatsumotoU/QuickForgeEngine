#pragma once

#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief Transform の位置を中心とする部屋の範囲と、その部屋を見るカメラ位置。
	struct CameraRoomComponent {
		// 部屋の軸はワールド軸に固定し、Transform の回転と拡縮は適用しない。
		QFE::MATH::Vector3 halfExtents{ 10.0f, 100.0f, 10.0f };
		QFE::MATH::Vector3 cameraOffset{ 0.0f, 16.8f, -11.7f };

		QFE_REFLECT_BEGIN(CameraRoomComponent)
			QFE_REFLECT_MEMBER(halfExtents)
			QFE_REFLECT_MEMBER(cameraOffset)
		QFE_REFLECT_END()
	};
}
