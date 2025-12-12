#include "AnimCrip.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
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
	// keyframeが存在しない場合はデフォルトのTransformを返す
	if (keyframes_.empty()) {
#ifdef _DEBUG
		DebugLog("GetTransformAtTime: No keyframes available.");
#endif // _DEBUG
		return result;
	}

	// 総アニメーション時間を計算
	float totalDuration = keyframes_.back().time;
	// ループ設定に基づいて時間を調整
	if (isLoop_) {
		time = fmod(time, totalDuration);
	} else {
		if (time >= totalDuration) {
			return keyframes_.back().transform;
		}
	}
	// 現在の時間に対応するキーフレームを見つける
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
	// 最初のキーフレームより前の場合
	if (!previousKeyFrame) {
		return keyframes_.front().transform;
	}
	// キーフレーム間の補間
	float segmentDuration = nextKeyFrame->time - previousKeyFrame->time;
	float t = (time - previousKeyFrame->time) / segmentDuration;
	t;

	return result;
}
