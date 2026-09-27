#pragma once

#include <cstdint>
#include <functional>

#include "math/MathInclude.h"

namespace QFE {
	class EntityManager;
}

namespace QFE::FRAMEWORK {
	using SplineLineDrawer = std::function<void(
		const QFE::MATH::Vector3&,
		const QFE::MATH::Vector3&,
		const QFE::MATH::Vector4&)>;

	/// @brief エンティティを制御点に沿って移動させる。
	void UpdateSplineMovement(
		QFE::EntityManager& entityManager,
		float deltaTime);

	/// @brief スプラインの制御点をワールド空間の線として描画する。
	void DrawSplineMovementPaths(
		const QFE::EntityManager& entityManager,
		const SplineLineDrawer& drawLine);
}
