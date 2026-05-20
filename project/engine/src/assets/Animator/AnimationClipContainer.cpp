#include "engine/include/assets/Animator/AnimationClipContainer.h"
#include "engine/include/core/EngineDefines.h"

using namespace QFE;

/** @brief 初期化 */
void AnimationClipContainer::Initialize(size_t containerSize) {
	animationClips_.clear();
	animationClips_.reserve(containerSize);
	containerSize_ = containerSize;
	nextAnimationId_ = 0;
}

/** @brief 終了処理 */
void AnimationClipContainer::Finalize() {
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("Finalizing AnimationClipContainer. Total registered animation clips: " + std::to_string(animationClips_.size()));
	for(const auto& clip : animationClips_) {
		QFE_LOG("Animation ID: " + std::to_string(clip.first) + ", Clip Name: " + clip.second.GetName() + ", KeyFrame Count: " + std::to_string(clip.second.GetKeyFrameCount()));
	}
#endif

	animationClips_.clear();
}

uint32_t QFE::AnimationClipContainer::RegisterAnimationClip(AnimClip animClip){
	animationClips_[nextAnimationId_] = std::move(animClip);
	return nextAnimationId_++;
}

const AnimClip& QFE::AnimationClipContainer::GetAnimationClip(uint32_t animationId) const{
	auto it = animationClips_.find(animationId);
	if (it != animationClips_.end()) {
		return it->second;
	} else {
		QFE_REPORT_SYSTEM_ERROR("AnimationClipContainer: Animation ID " + std::to_string(animationId) + " not found. Returning dummy clip.",SystemError::Abort);
		return dummyClip_;
	}
}