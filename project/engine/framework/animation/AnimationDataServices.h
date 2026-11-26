#pragma once

#include <string>

#include "animation/AnimationClip.h"
#include "components/AnimationComponent.h"

namespace QFE::FRAMEWORK {
	// クリップとキーフレームはデータ型として共有し、操作の窓口をここにまとめる。
	// 追加時の並べ替えや同時刻の扱いは AnimationClip の既存の動作を維持する。
	ANIMATION::AnimationClip CreateAnimationClip(const std::string& name, bool loop);
	void AddAnimationKeyFrame(ANIMATION::AnimationClip& clip, const ANIMATION::AnimationKeyFrame& keyFrame);
	const std::string& GetAnimationClipName(const ANIMATION::AnimationClip& clip);
	bool IsAnimationClipLooping(const ANIMATION::AnimationClip& clip);
	/// 返した参照はクリップが生存し、キーフレームが変更されない間だけ有効。
	const std::vector<ANIMATION::AnimationKeyFrame>& GetAnimationKeyFrames(const ANIMATION::AnimationClip& clip);
	float GetAnimationClipDuration(const ANIMATION::AnimationClip& clip);
	/// 指定秒数の姿勢を補間する。範囲外やループの扱いは元のクリップに従う。
	MATH::EulerTransform SampleAnimationClip(const ANIMATION::AnimationClip& clip, float time);

	/// @brief 旧アニメーションエディタと互換性のある.anim形式で保存する。
	bool SaveAnimationClip(const ANIMATION::AnimationClip& clip, const std::string& filePath);
	bool LoadAnimationClip(const std::string& filePath, ANIMATION::AnimationClip& clip);
	/// @brief コンポーネントの clipName を現在のアセット配下の .anim パスへ解決する。
	std::string ResolveAnimationClipPath(const std::string& clipName);
}
