#include "AnimCrip.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

AnimCrip::AnimCrip() : isLoop_(false){
}

void AnimCrip::SetLoop(bool isLoop) {
	isLoop_ = isLoop;
}

bool AnimCrip::IsLoop() const {
	return isLoop_;
}

void AnimCrip::AddKeyFrame(const KeyFrame& keyframe) {
	keyframes_.push_back(keyframe);
}

const std::vector<KeyFrame>& AnimCrip::GetKeyFrames() const {
	return keyframes_;
}

size_t AnimCrip::GetKeyFrameCount() const {
	return keyframes_.size();
}

Transform AnimCrip::GetTransformAtTime(float time) const {
	Transform result;
	// keyframe縺悟ｭ伜惠縺励↑縺・ｴ蜷医・繝・ヵ繧ｩ繝ｫ繝医・Transform繧定ｿ斐☆
	if (keyframes_.empty()) {
#ifdef _DEBUG
		DebugLog("GetTransformAtTime: No keyframes available.");
#endif // _DEBUG
		return result;
	}

	// 邱上い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ譎る俣繧定ｨ育ｮ・
	float totalDuration = keyframes_.back().time;
	// 繝ｫ繝ｼ繝苓ｨｭ螳壹↓蝓ｺ縺･縺・※譎る俣繧定ｪｿ謨ｴ
	if (isLoop_) {
		time = fmod(time, totalDuration);
	} else {
		if (time >= totalDuration) {
			return keyframes_.back().transform;
		}
	}
	// 迴ｾ蝨ｨ縺ｮ譎る俣縺ｫ蟇ｾ蠢懊☆繧九く繝ｼ繝輔Ξ繝ｼ繝繧定ｦ九▽縺代ｋ
	const KeyFrame* previousKeyFrame = nullptr;
	const KeyFrame* nextKeyFrame = nullptr;
	for (size_t i = 0; i < keyframes_.size(); ++i) {
		if (keyframes_[i].time >= time) {
			nextKeyFrame = &keyframes_[i];
			if (i > 0) {
				previousKeyFrame = &keyframes_[i - 1];
			}
			break;
		}
	}
	// 譛蛻昴・繧ｭ繝ｼ繝輔Ξ繝ｼ繝繧医ｊ蜑阪・蝣ｴ蜷・
	if (!previousKeyFrame) {
		return keyframes_.front().transform;
	}
	// 繧ｭ繝ｼ繝輔Ξ繝ｼ繝髢薙・陬憺俣
	float segmentDuration = nextKeyFrame->time - previousKeyFrame->time;
	float t = (time - previousKeyFrame->time) / segmentDuration;
	t;

	return result;
}
