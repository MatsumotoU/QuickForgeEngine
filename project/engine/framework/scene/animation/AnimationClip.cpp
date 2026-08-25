#include "AnimationClip.h"

#include <algorithm>
#include <cmath>

void QFE::ANIMATION::AnimationClip::AddKeyFrame(const AnimationKeyFrame& keyFrame) {
	keyFrames_.push_back(keyFrame);
	std::stable_sort(keyFrames_.begin(), keyFrames_.end(),
		[](const AnimationKeyFrame& lhs, const AnimationKeyFrame& rhs) {
			return lhs.time < rhs.time;
		});
}

void QFE::ANIMATION::AnimationClip::ClearKeyFrames() {
	keyFrames_.clear();
}

float QFE::ANIMATION::AnimationClip::GetDuration() const {
	return keyFrames_.empty() ? 0.0f : keyFrames_.back().time;
}

QFE::MATH::EulerTransform QFE::ANIMATION::AnimationClip::Sample(float time) const {
	if (keyFrames_.empty()) return {};
	const float duration = GetDuration();
	if (loop_ && duration > 0.0f) {
		time = std::fmod((std::max)(0.0f, time), duration);
	} else {
		time = std::clamp(time, 0.0f, duration);
	}

	const auto next = std::lower_bound(keyFrames_.begin(), keyFrames_.end(), time,
		[](const AnimationKeyFrame& keyFrame, float sampleTime) {
			return keyFrame.time < sampleTime;
		});
	if (next == keyFrames_.begin()) return next->transform;
	if (next == keyFrames_.end()) return keyFrames_.back().transform;

	const AnimationKeyFrame& previousKey = *(next - 1);
	const AnimationKeyFrame& nextKey = *next;
	const float span = nextKey.time - previousKey.time;
	const float amount = span > 0.0f ? (time - previousKey.time) / span : 0.0f;

	MATH::EulerTransform result;
	result.translate = MATH::Vector3::Lerp(
		previousKey.transform.translate, nextKey.transform.translate, amount);
	result.rotate = MATH::Vector3::Lerp(
		previousKey.transform.rotate, nextKey.transform.rotate, amount);
	result.scale = MATH::Vector3::Lerp(
		previousKey.transform.scale, nextKey.transform.scale, amount);
	return result;
}
