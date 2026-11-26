#pragma once

#include "math/MathInclude.h"
#include <algorithm>
#include <cmath>

namespace QFE::EDITOR {
	enum class GridPlane { XZ, XY, YZ };

	/// 配置面と、原点から法線方向へ何マス移すかをシーンビューで共有する。
	struct GridPlacement {
		bool enabled = false;
		GridPlane plane = GridPlane::XZ;
		float spacing = 1.0f;
		int layer = 0;
	};

	inline float SnapGridCoordinate(float value, float spacing) {
		return std::round(value / spacing) * spacing;
	}

	inline MATH::Vector3 SnapGridPosition(MATH::Vector3 position, float spacing) {
		position.x = SnapGridCoordinate(position.x, spacing);
		position.y = SnapGridCoordinate(position.y, spacing);
		position.z = SnapGridCoordinate(position.z, spacing);
		return position;
	}

	/// ビューポート内の座標から配置面への交点を求め、平面内だけ格子点に合わせる。
	inline bool IntersectGrid(const GridPlacement& grid, const MATH::Matrix4x4& viewProjection,
		float x, float y, MATH::Vector3& result) {
		if (!std::isfinite(grid.spacing) || grid.spacing <= 0.0f) return false;
		const auto inverse = viewProjection.Inverse();
		const auto nearPoint = MATH::Vector3::EulerTransform({ x * 2.0f - 1.0f, 1.0f - y * 2.0f, 0.0f }, inverse);
		const auto farPoint = MATH::Vector3::EulerTransform({ x * 2.0f - 1.0f, 1.0f - y * 2.0f, 1.0f }, inverse);
		const auto direction = farPoint - nearPoint;
		const float origin = grid.plane == GridPlane::XZ ? nearPoint.y : grid.plane == GridPlane::XY ? nearPoint.z : nearPoint.x;
		const float normal = grid.plane == GridPlane::XZ ? direction.y : grid.plane == GridPlane::XY ? direction.z : direction.x;
		const float height = static_cast<float>(grid.layer) * grid.spacing;
		// 平行・カメラの背後・遠平面より外への配置は、誤った位置に置かない。
		if (!std::isfinite(normal) || std::fabs(normal) < 1.0e-6f) return false;
		const float t = (height - origin) / normal;
		if (!std::isfinite(t) || t < 0.0f || t > 1.0f) return false;
		result = SnapGridPosition(nearPoint + direction * t, grid.spacing);
		if (grid.plane == GridPlane::XZ) result.y = height;
		else if (grid.plane == GridPlane::XY) result.z = height;
		else result.x = height;
		return std::isfinite(result.x) && std::isfinite(result.y) && std::isfinite(result.z);
	}
}
