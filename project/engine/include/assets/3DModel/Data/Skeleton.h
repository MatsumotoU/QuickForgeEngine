#pragma once
#include "engine/include/core/Math/Transform.h"
#include <string>
#include <vector>
#include <optional>
#include <map>

namespace QFE
{
	struct Joint
	{
		int32_t enmityId; ///< ジョイントのインデックス
		std::optional<int32_t> parentId; ///< 親ジョイントのインデックス（オプション）
		std::string name; ///< ジョイントの名前
	};

	struct Skeleton
	{
		int32_t rootJointIndex; ///< ルートジョイントのインデックス
		std::map<std::string, int32_t> jointMap; ///< ジョイント名とインデックスのマップ
		std::vector<Joint> joints; ///< ジョイントのリスト
	};
}
