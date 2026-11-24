#pragma once

#include "math/transform/Transform.h"
#include <cmath>

namespace QFE::FRAMEWORK::ColliderDebugDraw {
	// Collider sizes are in world units, matching the shooting collision system.
	template<class DrawLine>
	void Box(const MATH::EulerTransform& transform, const MATH::Vector3& halfExtents,
		bool oriented, const DrawLine& drawLine) {
		const auto rotation = oriented ? MATH::Matrix4x4::MakeRotateXYZMatrix(transform.rotate)
			: MATH::Matrix4x4::MakeIdentity4x4();
		const MATH::Vector3 axes[3] = {
			{ rotation.m[0][0], rotation.m[0][1], rotation.m[0][2] },
			{ rotation.m[1][0], rotation.m[1][1], rotation.m[1][2] },
			{ rotation.m[2][0], rotation.m[2][1], rotation.m[2][2] }
		};
		const float extents[3] = { std::abs(halfExtents.x), std::abs(halfExtents.y), std::abs(halfExtents.z) };
		MATH::Vector3 corners[8];
		for (int i = 0; i < 8; ++i) {
			corners[i] = transform.translate;
			for (int axis = 0; axis < 3; ++axis) {
				corners[i] += axes[axis] * ((i & (1 << axis)) ? extents[axis] : -extents[axis]);
			}
		}
		for (int i = 0; i < 8; ++i) {
			for (int axis = 0; axis < 3; ++axis) {
				if ((i & (1 << axis)) == 0) drawLine(corners[i], corners[i | (1 << axis)]);
			}
		}
	}

	template<class DrawLine>
	void Sphere(const MATH::Vector3& center, float radius, const DrawLine& drawLine) {
		constexpr int segments = 16;
		constexpr float tau = 6.28318530717958647692f;
		const float r = std::abs(radius);
		for (int plane = 0; plane < 3; ++plane) {
			auto point = [&](int index) {
				const float angle = tau * static_cast<float>(index) / static_cast<float>(segments);
				const float x = r * std::cos(angle), y = r * std::sin(angle);
				if (plane == 0) return center + MATH::Vector3{ x, y, 0.0f };
				if (plane == 1) return center + MATH::Vector3{ x, 0.0f, y };
				return center + MATH::Vector3{ 0.0f, x, y };
			};
			for (int i = 0; i < segments; ++i) drawLine(point(i), point(i + 1));
		}
	}
}
