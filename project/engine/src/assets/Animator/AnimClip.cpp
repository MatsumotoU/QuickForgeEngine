#include "engine/include/assets/Animator/AnimClip.h"
#include "engine/include/core/EngineDefines.h"

using namespace QFE;

void AnimClip::SetName(const std::string& name) {
	name_ = name;
}

const std::string& AnimClip::GetName() const {
	return name_;
}

void AnimClip::SetLoop(bool isLoop) {
	isLoop_ = isLoop;
}

bool AnimClip::IsLoop() const {
	return isLoop_;
}

void AnimClip::AddKeyFrame(const KeyFrame& keyframe) {
	keyframes_.push_back(keyframe);
}

std::vector<KeyFrame> AnimClip::GetKeyFrames() const {
	std::vector<KeyFrame> result(keyframes_.begin(), keyframes_.end());
	return result;
}

size_t AnimClip::GetKeyFrameCount() const {
	return keyframes_.size();
}

Transform AnimClip::GetTransformAtTime(float time) const {
	Transform result;
	// キーフレームが存在しない場合はデフォルトのTransformを返す
	if (keyframes_.empty()) {
		QFE_LOG("GetTransformAtTime: No keyframes available.");
		return result;
	}

	// アニメーションの総時間を取得
	float totalDuration = keyframes_.back().time;
	// ループする場合は時間を総時間で割った余りにする
	if (isLoop_) {
		time = fmod(time, totalDuration);
	} else {
		if (time >= totalDuration) {
			return keyframes_.back().transform;
		}
	}
	// 時間に基づいて前後のキーフレームを見つける
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
	// 前のキーフレームが存在しない場合は次のキーフレームのTransformを返す
	if (!previousKeyFrame) {
		return keyframes_.front().transform;
	}
	// 次のキーフレームが存在しない場合は前のキーフレームのTransformを返す
	if (!nextKeyFrame) {
		return keyframes_.back().transform;
	}
	// 前後のキーフレームのTransformを線形補間する
	float segmentDuration = nextKeyFrame->time - previousKeyFrame->time;
	float t = (time - previousKeyFrame->time) / segmentDuration;

	result.translate = Vector3::Lerp(previousKeyFrame->transform.translate, nextKeyFrame->transform.translate, t);

	return result;
}

float QFE::AnimClip::GetTotalDuration() const
{
	if (keyframes_.empty()) {
		return 0.0f;
	}
	return keyframes_.back().time;
}
