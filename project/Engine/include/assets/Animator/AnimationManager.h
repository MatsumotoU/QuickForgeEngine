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

	Transform GetAnimationTransform(uint32_t animationId, float currentTime);

private:
	std::unordered_map<uint32_t, AnimClip> animationMap_;
};
