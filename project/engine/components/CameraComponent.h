#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief カメラの情報を表すコンポーネントです。
	struct CameraComponent {
		float fovY_;
		float aspectRatio_;
		float nearZ_;
		float farZ_;
		// 画面の左、右、上、下の座標
		float left_;
		float right_;
		float top_;
		float bottom_;

		MATH::Transform transform;
		MATH::Matrix4x4 viewMatrix;
		MATH::Matrix4x4 projectionMatrix;
		bool isMainCamera = false;

		QFE_REFLECT_BEGIN(CameraComponent)
			QFE_REFLECT_MEMBER(fovY_)
			QFE_REFLECT_MEMBER(aspectRatio_)
			QFE_REFLECT_MEMBER(nearZ_)
			QFE_REFLECT_MEMBER(farZ_)
			QFE_REFLECT_MEMBER(left_)
			QFE_REFLECT_MEMBER(right_)
			QFE_REFLECT_MEMBER(top_)
			QFE_REFLECT_MEMBER(bottom_)
			QFE_REFLECT_MEMBER(transform)
			QFE_REFLECT_END()
	};

	QFE_COMPONENT(CameraComponent)
}