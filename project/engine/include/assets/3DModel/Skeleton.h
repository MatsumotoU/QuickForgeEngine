#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "engine/include/core/Math/Matrix/Matrix4x4.h"

struct aiNode;

namespace QFE::SKELETON {

	struct Joint {
		std::string name;
		int32_t parentIndex = -1;
		Matrix4x4 localMatrix = Matrix4x4::MakeIndentity4x4();
		Matrix4x4 globalMatrix = Matrix4x4::MakeIndentity4x4();
		Matrix4x4 inverseBindMatrix = Matrix4x4::MakeIndentity4x4();
		std::vector<int32_t> children;
	};

	struct Skeleton {
		std::vector<Joint> joints;
		std::unordered_map<std::string, int32_t> jointIndexMap;
		int32_t rootIndex = -1;
	};

	/// @brief ルートノードからスケルトンを構築する
	Skeleton CreateSkeleton(const aiNode* rootNode);

	/// @brief ノードを1つJointとして登録し、子を再帰的に構築する
	int32_t CreateJoint(const aiNode* node, int32_t parentIndex, Skeleton& skeleton);

	/// @brief スケルトンのグローバル行列を更新する
	void Update(Skeleton& skeleton);

}