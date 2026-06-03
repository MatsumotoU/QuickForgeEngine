#pragma once
#include "engine/include/assets/Animator/AnimClip.h"
#include "engine/include/assets/3DModel/Skeleton.h"

namespace QFE::ANIMATION {

	/// @brief AnimClipをSkeletonのルートに適用し、スケルトンを更新する
	void ApplyAnimation(QFE::SKELETON::Skeleton& skeleton, const AnimClip& animClip, float time);

}