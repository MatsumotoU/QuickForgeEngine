#pragma once
#include <vector>
#include <string>
#include "KeyFrame.h"

class AnimClip final {
public:
	explicit AnimClip(const std::string& name);
	void SetLoop(bool isLoop);
	bool IsLoop() const;
	void AddKeyFrame(const KeyFrame& keyframe);
	const std::vector<KeyFrame>& GetKeyFrames() const;
	size_t GetKeyFrameCount() const;
	Transform GetTransformAtTime(float time) const;

private:
	bool isLoop_;
	const std::string name_;
	std::vector<KeyFrame> keyframes_;
};
