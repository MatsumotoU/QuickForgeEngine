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

	/// @brief Moves an entity along its spline control points.
	void UpdateSplineMovement(
		QFE::EntityManager& entityManager,
		float deltaTime);

	/// @brief Draws spline paths and their looping triangle movement preview.
	void DrawSplineMovementPaths(
		const QFE::EntityManager& entityManager,
		const SplineLineDrawer& drawLine);
}
