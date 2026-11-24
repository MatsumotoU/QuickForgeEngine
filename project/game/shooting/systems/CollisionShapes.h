#pragma once

#include "math/transform/Transform.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace QFE::GAMESYSTEM::CollisionShapes {
	using MATH::Vector3;
	struct Shape {
		uint32_t entityId;
		uint32_t mask;
		bool sphere;
		Vector3 center;
		float radius;
		Vector3 axes[3];
		float extents[3];
	};

	inline float Dot(const Vector3& a, const Vector3& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	inline Vector3 Cross(const Vector3& a, const Vector3& b) {
		return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	}
	inline Shape MakeBox(uint32_t id, uint32_t mask, const MATH::EulerTransform& transform,
		const Vector3& halfExtents, bool oriented) {
		const auto rotation = oriented ? MATH::Matrix4x4::MakeRotateXYZMatrix(transform.rotate)
			: MATH::Matrix4x4::MakeIdentity4x4();
		return { id, mask, false, transform.translate, 0.0f,
			{ { rotation.m[0][0], rotation.m[0][1], rotation.m[0][2] },
			  { rotation.m[1][0], rotation.m[1][1], rotation.m[1][2] },
			  { rotation.m[2][0], rotation.m[2][1], rotation.m[2][2] } },
			{ std::abs(halfExtents.x), std::abs(halfExtents.y), std::abs(halfExtents.z) } };
	}
	inline bool SphereBox(const Shape& sphere, const Shape& box) {
		const Vector3 offset = sphere.center - box.center;
		float distanceSquared = 0.0f;
		for (int i = 0; i < 3; ++i) {
			const float coordinate = Dot(offset, box.axes[i]);
			const float outside = coordinate - std::clamp(coordinate, -box.extents[i], box.extents[i]);
			distanceSquared += outside * outside;
		}
		return distanceSquared < sphere.radius * sphere.radius;
	}
	inline bool Separated(const Shape& a, const Shape& b, const Vector3& axis) {
		// Parallel box axes produce a zero cross product, which cannot separate them.
		if (Dot(axis, axis) < 1.0e-12f) return false;
		float projectedA = 0.0f, projectedB = 0.0f;
		for (int i = 0; i < 3; ++i) {
			projectedA += a.extents[i] * std::abs(Dot(a.axes[i], axis));
			projectedB += b.extents[i] * std::abs(Dot(b.axes[i], axis));
		}
		return std::abs(Dot(b.center - a.center, axis)) >= projectedA + projectedB;
	}
	inline bool Intersects(const Shape& a, const Shape& b) {
		if (a.sphere && b.sphere) {
			const Vector3 offset = a.center - b.center;
			const float radii = a.radius + b.radius;
			return radii > 0.0f && Dot(offset, offset) < radii * radii;
		}
		if (a.sphere) return SphereBox(a, b);
		if (b.sphere) return SphereBox(b, a);
		for (int i = 0; i < 3; ++i) {
			if (Separated(a, b, a.axes[i]) || Separated(a, b, b.axes[i])) return false;
			for (int j = 0; j < 3; ++j) {
				if (Separated(a, b, Cross(a.axes[i], b.axes[j]))) return false;
			}
		}
		return true;
	}
}
