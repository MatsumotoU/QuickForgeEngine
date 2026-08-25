#pragma once

#include <string>

#include "AnimationClip.h"

namespace QFE::ANIMATION {
	/// @brief 旧アニメーションエディタと互換性のある.anim形式で保存する。
	bool SaveAnimationClip(const AnimationClip& clip, const std::string& filePath);
	bool LoadAnimationClip(const std::string& filePath, AnimationClip& clip);
	/// @brief コンポーネントのclipNameをresources配下の.animパスへ解決する。
	std::string ResolveAnimationClipPath(const std::string& clipName);
}
