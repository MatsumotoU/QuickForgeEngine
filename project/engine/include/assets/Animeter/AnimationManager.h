#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include <unordered_map>
#include "string"
#include "AnimCrip.h" 

class AnimationManager final : public Singleton<AnimationManager> {
	friend class Singleton<AnimationManager>;
public:
	void Initalize();
	void Update();
	void Finalize();

	Transform GetAnimationTransform(uint32_t animationId, float currentTime);

private:
	std::unordered_map<uint32_t, AnimCrip> animationMap_;
};