#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include <unordered_map>
#include "string"
#include "AnimClip.h" 

class AnimationManager final : public Singleton<AnimationManager> {
	friend class Singleton<AnimationManager>;
public:
	void Initialize();
	void Update();
	void Finalize();

	// 空のアニメーションクリップを作成
	uint32_t CreateEmptyAnimationClip(const std::string& name);
	// アニメーションクリップを登録
	uint32_t RegisterAnimationClip(uint32_t animationId, const AnimClip& animClip);
	// アニメーションクリップを取得
	AnimClip* GetAnimationClip(uint32_t animationId);

private:
	uint32_t nextAnimationId_;
	std::unordered_map<uint32_t, AnimClip> animationMap_;
};
