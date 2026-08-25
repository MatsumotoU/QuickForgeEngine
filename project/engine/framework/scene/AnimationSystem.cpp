#include "AnimationSystem.h"

#include "animation/AnimationDataServices.h"
#include "design-patterns/EntityManager.h"
#include "components/AnimationComponent.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <unordered_map>

namespace {
	struct CachedClip {
		QFE::ANIMATION::AnimationClip clip;
		std::filesystem::file_time_type lastWriteTime{};
		bool loaded = false;
	};

	const QFE::ANIMATION::AnimationClip* GetClip(const std::string& clipName) {
		static std::unordered_map<std::string, CachedClip> cache;
		const std::string path = QFE::ANIMATION::ResolveAnimationClipPath(clipName);
		if (path.empty()) return nullptr;

		std::error_code error;
		const auto writeTime = std::filesystem::last_write_time(path, error);
		if (error) return nullptr;
		CachedClip& entry = cache[path];
		if (!entry.loaded || entry.lastWriteTime != writeTime) {
			QFE::ANIMATION::AnimationClip loaded;
			if (!QFE::ANIMATION::LoadAnimationClip(path, loaded)) return nullptr;
			entry.clip = std::move(loaded);
			entry.lastWriteTime = writeTime;
			entry.loaded = true;
		}
		return &entry.clip;
	}
}

bool QFE::FRAMEWORK::PlayAnimation(EntityManager& entityManager, uint32_t entityId, bool restart) {
	auto* animation = entityManager.GetComponentPtr<SCENE::AnimationComponent>(entityId);
	if (animation == nullptr) return false;
	animation->requestFlags |= restart
		? SCENE::AnimationRequestRestart
		: SCENE::AnimationRequestPlay;
	return true;
}

bool QFE::FRAMEWORK::StopAnimation(EntityManager& entityManager, uint32_t entityId, bool resetTime) {
	auto* animation = entityManager.GetComponentPtr<SCENE::AnimationComponent>(entityId);
	if (animation == nullptr) return false;
	animation->requestFlags |= SCENE::AnimationRequestStop;
	if (resetTime) animation->currentTime = 0.0f;
	return true;
}

void QFE::FRAMEWORK::UpdateAnimationComponents(EntityManager& entityManager, float deltaTime) {
	entityManager.Each<SCENE::AnimationComponent>(
		[](uint32_t, SCENE::AnimationComponent& animation) {
			if (!animation.initialized) {
				animation.initialized = true;
				animation.playing = animation.playOnStart;
				animation.currentTime = 0.0f;
			}

			const uint32_t requests = animation.requestFlags;
			animation.requestFlags = SCENE::AnimationRequestNone;
			if ((requests & SCENE::AnimationRequestRestart) != 0) {
				animation.currentTime = 0.0f;
				animation.playing = true;
			} else {
				if ((requests & SCENE::AnimationRequestToggle) != 0) animation.playing = !animation.playing;
				if ((requests & SCENE::AnimationRequestPlay) != 0) animation.playing = true;
				if ((requests & SCENE::AnimationRequestStop) != 0) animation.playing = false;
			}
		});

	entityManager.Each<SCENE::AnimationComponent>(
		[deltaTime](uint32_t, SCENE::AnimationComponent& animation) {
			const ANIMATION::AnimationClip* clip = GetClip(animation.clipName);
			if (clip == nullptr || clip->GetKeyFrames().empty()) {
				animation.playing = false;
				return;
			}
			const float duration = clip->GetDuration();
			if (animation.playing) {
				animation.currentTime += (std::max)(0.0f, deltaTime) *
					(std::max)(0.0f, animation.playbackSpeed);
				if (duration <= 0.0f) {
					animation.currentTime = 0.0f;
					animation.playing = false;
				} else if (animation.useClipLoop ? clip->IsLoop() : animation.loop) {
					animation.currentTime = std::fmod(animation.currentTime, duration);
				} else if (animation.currentTime >= duration) {
					animation.currentTime = duration;
					animation.playing = false;
				}
			}
			animation.transform = clip->Sample(animation.currentTime);
		});
}
