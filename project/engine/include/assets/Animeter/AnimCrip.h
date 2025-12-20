#pragma once
#include <vector>
#include "KeyFrame.h"

class AnimCrip final {
public:
	AnimCrip();
	void SetLoop(bool isLoop);
	bool IsLoop() const;
	void AddKeyFrame(const KeyFrame& keyframe);
	const std::vector<KeyFrame>& GetKeyFrames() const;
	size_t GetKeyFrameCount() const;

	Transform GetTransformAtTime(float time) const;

private:
	bool isLoop_;
	std::vector<KeyFrame> keyframes_;
};
