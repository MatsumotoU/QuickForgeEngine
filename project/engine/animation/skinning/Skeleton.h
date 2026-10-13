#pragma once
#include "Joint.h"
#include <map>	

namespace QFE::GRAPHIC {
	/// @brief スケルトンのジョイントを表す構造体
	struct Skeleton {
		int32_t rootJointIndex; // ルートジョイントのインデックス
		std::map<std::string, int32_t> jointNameToIndexMap; // ジョイント名からインデックスへのマップ
		std::vector<Joint> joints; // ジョイントの配列
	};
}