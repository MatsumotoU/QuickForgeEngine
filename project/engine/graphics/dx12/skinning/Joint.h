#pragma once
#include <string>
#include <vector>
#include <optional>
#include "math/MathInclude.h"

namespace QFE::GRAPHIC {
	/// @brief スケルトンのジョイントを表す構造体
	struct Joint {
		QFE::MATH::QuaternionTransform transform; // ジョイントの変換情報（位置、回転、スケール）
		QFE::MATH::Matrix4x4 localMatrix; // ジョイントのローカル変換行列
		QFE::MATH::Matrix4x4 skeletonSpaceMatrix; // ジョイントのスケルトンスペース変換行列
		std::string name; // ジョイントの名前
		std::vector<int32_t> children; // 子ジョイントのインデックスの配列
		int32_t index; // ジョイントのインデックス
		std::optional<int32_t> parentIndex; // 親ジョイントのインデックス（存在しない場合はnullopt）
	};
}