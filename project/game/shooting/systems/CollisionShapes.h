#pragma once

#include "math/transform/Transform.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

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

	// 回転や壁の移動で既に重なった箱を、最も短い距離で外へ出す。
	inline bool BoxPenetration(const Shape& moving, const Shape& wall, float& depth, Vector3& normal) {
		float smallest = (std::numeric_limits<float>::max)();
		Vector3 best{};
		const Vector3 offset = moving.center - wall.center;
		const auto testAxis = [&](Vector3 axis) {
			const float lengthSquared = Dot(axis, axis);
			if (lengthSquared < 1.0e-12f) return true;
			axis = axis * (1.0f / std::sqrt(lengthSquared));
			float extent = 0.0f;
			for (int i = 0; i < 3; ++i)
				extent += moving.extents[i] * std::abs(Dot(moving.axes[i], axis)) +
					wall.extents[i] * std::abs(Dot(wall.axes[i], axis));
			const float distance = Dot(offset, axis);
			const float overlap = extent - std::abs(distance);
			if (overlap <= 1.0e-6f) return false;
			if (overlap < smallest) {
				smallest = overlap;
				best = distance < 0.0f ? axis * -1.0f : axis;
			}
			return true;
		};
		for (int i = 0; i < 3; ++i) {
			if (!testAxis(moving.axes[i]) || !testAxis(wall.axes[i])) return false;
			for (int j = 0; j < 3; ++j)
				if (!testAxis(Cross(moving.axes[i], wall.axes[j]))) return false;
		}
		depth = smallest;
		normal = best;
		return true;
	}

	// 球の中心を半径分広げた箱に対して掃引し、高速移動でも壁を飛び越えないようにする。
	// 角では実際の球より保守的に止まるが、壁へのめり込みを防げる。
	inline bool SweepSphereBox(const Vector3& center, float radius, const Vector3& movement,
		const Shape& box, float& time, Vector3& normal) {
		const Vector3 offset = center - box.center;
		float enter = 0.0f, exit = 1.0f;
		Vector3 hitNormal{};
		bool inside = true;
		for (int i = 0; i < 3; ++i) {
			const float position = Dot(offset, box.axes[i]);
			const float speed = Dot(movement, box.axes[i]);
			// Windows の min/max マクロに展開されないよう関数名を括る。
			const float extent = box.extents[i] + (std::max)(radius, 0.0f);
			if (std::abs(position) >= extent - 1.0e-5f) inside = false;
			if (std::abs(speed) < 1.0e-8f) {
				if (position <= -extent || position >= extent) return false;
				continue;
			}
			float nearTime = (-extent - position) / speed;
			float farTime = (extent - position) / speed;
			Vector3 nearNormal = box.axes[i] * -1.0f;
			if (nearTime > farTime) {
				std::swap(nearTime, farTime);
				nearNormal = box.axes[i];
			}
			if (nearTime >= enter) { enter = nearTime; hitNormal = nearNormal; }
			exit = (std::min)(exit, farTime);
			if (enter > exit) return false;
		}
		// 初期配置ですでに重なっている場合は脱出を妨げない。
		if (inside || enter < 0.0f || enter > 1.0f || Dot(movement, hitNormal) >= 0.0f) return false;
		time = enter;
		normal = hitNormal;
		return true;
	}

	// 箱を平行移動させたとき、各分離軸で重なり始める時間を求める。
	// 回転は移動前の姿勢で固定し、移動中に薄い壁を通り抜けることを防ぐ。
	inline bool SweepBoxBox(const Shape& moving, const Vector3& movement,
		const Shape& wall, float& time, Vector3& normal) {
		float enter = 0.0f, exit = 1.0f;
		Vector3 hitNormal{};
		bool inside = true;
		const auto testAxis = [&](Vector3 axis) {
			const float lengthSquared = Dot(axis, axis);
			// 平行な辺の外積は分離軸にならない。
			if (lengthSquared < 1.0e-12f) return true;
			axis = axis * (1.0f / std::sqrt(lengthSquared));
			float extent = 0.0f;
			for (int i = 0; i < 3; ++i)
				extent += moving.extents[i] * std::abs(Dot(moving.axes[i], axis)) +
					wall.extents[i] * std::abs(Dot(wall.axes[i], axis));
			const float position = Dot(moving.center - wall.center, axis);
			const float speed = Dot(movement, axis);
			if (std::abs(position) >= extent - 1.0e-5f) inside = false;
			if (std::abs(speed) < 1.0e-8f) return position > -extent && position < extent;
			float nearTime = (-extent - position) / speed;
			float farTime = (extent - position) / speed;
			Vector3 nearNormal = axis * -1.0f;
			if (nearTime > farTime) { std::swap(nearTime, farTime); nearNormal = axis; }
			if (nearTime >= enter) { enter = nearTime; hitNormal = nearNormal; }
			exit = (std::min)(exit, farTime);
			return enter <= exit;
		};
		for (int i = 0; i < 3; ++i) {
			if (!testAxis(moving.axes[i]) || !testAxis(wall.axes[i])) return false;
			for (int j = 0; j < 3; ++j)
				if (!testAxis(Cross(moving.axes[i], wall.axes[j]))) return false;
		}
		// 既に重なっている場合は、球と同様に脱出を妨げない。
		if (inside || enter < 0.0f || enter > 1.0f || Dot(movement, hitNormal) >= 0.0f) return false;
		time = enter;
		normal = hitNormal;
		return true;
	}
}
