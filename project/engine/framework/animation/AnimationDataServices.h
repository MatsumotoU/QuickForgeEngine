#pragma once

#include <string>

#include "animation/AnimationClip.h"

namespace QFE::FRAMEWORK {
	/// @brief 旧アニメーションエディタと互換性のある.anim形式で保存する。
	bool SaveAnimationClip(const ANIMATION::AnimationClip& clip, const std::string& filePath);
	bool LoadAnimationClip(const std::string& filePath, ANIMATION::AnimationClip& clip);
	/// @brief コンポーネントのclipNameをresources配下の.animパスへ解決する。
	std::string ResolveAnimationClipPath(const std::string& clipName);
}
