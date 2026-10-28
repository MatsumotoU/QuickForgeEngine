#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <cmath>

namespace QFE::SCENE {
	/// ワールド空間での光の進行方向を表す平行光。
	struct DirectionalLightComponent {
		MATH::Vector3 direction{ 0.0f, -1.0f, 0.0f };
		float intensity = 1.0f;

		/// ゼロや非有限値は既定の下向きに戻す。
		static MATH::Vector3 NormalizeDirection(const MATH::Vector3& value) {
			const double length = std::hypot(static_cast<double>(value.x),
				static_cast<double>(value.y), static_cast<double>(value.z));
			if (!std::isfinite(length) || length < 1.0e-8) {
				return { 0.0f, -1.0f, 0.0f };
			}
			return { static_cast<float>(value.x / length), static_cast<float>(value.y / length),
				static_cast<float>(value.z / length) };
		}

		QFE_REFLECT_BEGIN(DirectionalLightComponent)
			if (ar.IsLoading() && !ar.HasField("direction") && ar.HasField("rotation")) {
				MATH::Vector3 rotation{};
				ar.Process("rotation", rotation);
				const auto matrix = MATH::Matrix4x4::MakeRotateXYZMatrix(rotation);
				direction = { -matrix.m[1][0], -matrix.m[1][1], -matrix.m[1][2] };
			} else {
				QFE_REFLECT_MEMBER(direction)
			}
			if (ar.IsLoading()) direction = NormalizeDirection(direction);
			QFE_REFLECT_MEMBER(intensity)
		QFE_REFLECT_END()
	};
}
