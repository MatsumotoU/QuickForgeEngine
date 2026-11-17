#pragma once
#include "EngineDefines.h"
#include <string>

namespace QFE::SCENE {
	/// @brief モデルに含まれるスケルトンアニメーションの再生を管理する。
	struct SkeletonAnimationComponent {
		std::string glbAnimationName; ///< 空の場合はGLBの先頭クリップを使用する。
		bool playGlbAnimation = true; ///< アニメーションの再生状態。
		bool loopGlbAnimation = true; ///< 終端から先頭へ戻して再生するか。
		float glbAnimationSpeed = 1.0f; ///< 再生速度の倍率。

		double glbAnimationTime = 0.0; ///< 再生位置。実行時のみ保持する。
		std::string activeGlbAnimationName; ///< 再生中のクリップ変更を検出する。

		QFE_REFLECT_BEGIN(SkeletonAnimationComponent)
			QFE_REFLECT_MEMBER(glbAnimationName)
			QFE_REFLECT_MEMBER(playGlbAnimation)
			QFE_REFLECT_MEMBER(loopGlbAnimation)
			QFE_REFLECT_MEMBER(glbAnimationSpeed)
		QFE_REFLECT_END()
	};
}
