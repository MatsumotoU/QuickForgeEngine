#pragma once
#include "Skeleton.h"
#include "Node.h"

namespace QFE::GRAPHIC {
	/// @brief Node構造体からJoint構造体を作成する関数
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parentIndex, std::vector<Joint>& joints);
	/// @brief Node構造体からSkeleton構造体を作成する関数
	Skeleton CreateSkeleton(const Node& rootNode);
	/// @brief Skeleton構造体のジョイントのスケルトンスペース行列を更新する関数
	void UpdateSkeletonSpaceMatrices(Skeleton& skeleton);
}