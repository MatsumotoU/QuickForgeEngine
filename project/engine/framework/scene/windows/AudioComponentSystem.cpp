#include "framework/scene/AudioComponentSystem.h"

#include "audio/AudioEngine.h"
#include "components/BGMComponent.h"
#include "components/SEComponent.h"
#include "scene/SceneManager.h"

#include <unordered_set>
#include <algorithm>

void QFE::FRAMEWORK::AudioComponentSystem::Reset(AUDIO::AudioEngine& audio) {
	// 他のシーンが所有する音声には触れない。
	for (const auto& [id, handle] : bgmVoices_) { (void)id; audio.Stop(handle); }
	for (const auto& [id, handle] : loopingSeVoices_) { (void)id; audio.Stop(handle); }
	for (const auto handle : oneShotVoices_) audio.Stop(handle);
	bgmVoices_.clear();
	loopingSeVoices_.clear();
	oneShotVoices_.clear();
	sceneRevision_ = 0;
	paused_ = false;
}

void QFE::FRAMEWORK::AudioComponentSystem::SetPaused(AUDIO::AudioEngine& audio, bool paused) {
	if (paused_ == paused) return;
	const auto setVoice = [&](uint32_t handle) {
		if (paused) audio.Pause(handle); else audio.Resume(handle);
	};
	for (const auto& [id, handle] : bgmVoices_) { (void)id; setVoice(handle); }
	for (const auto& [id, handle] : loopingSeVoices_) { (void)id; setVoice(handle); }
	for (const auto handle : oneShotVoices_) setVoice(handle);
	paused_ = paused;
}

void QFE::FRAMEWORK::AudioComponentSystem::Update(SCENE::SceneManager& scene, AUDIO::AudioEngine& audio) {
	Update(scene.GetCurrentSceneEntityManager(), scene.GetSceneRevision(), audio);
}

void QFE::FRAMEWORK::AudioComponentSystem::Update(
	EntityManager& entities, uint64_t sceneRevision, AUDIO::AudioEngine& audio) {
	const bool sceneStarted = sceneRevision_ != sceneRevision;
	if (sceneStarted) {
		Reset(audio);
		sceneRevision_ = sceneRevision;
	}

	std::unordered_set<uint32_t> activeBgm;
	entities.Each<SCENE::BGMComponent>([&](uint32_t id, SCENE::BGMComponent& bgm) {
		activeBgm.insert(id);
		const bool requested = bgm.isPlayReqest;
		bgm.isPlayReqest = false;
		if (!requested && !(sceneStarted && bgm.playOnSceneStart)) return;
		const auto old = bgmVoices_.find(id);
		if (old != bgmVoices_.end()) {
			audio.Stop(old->second);
			bgmVoices_.erase(old);
		}
		const uint32_t handle = audio.Play(bgm.audioPath, bgm.isLoop, bgm.volume, AUDIO::AudioCategory::BGM);
		if (handle) bgmVoices_[id] = handle;
	});
	for (auto it = bgmVoices_.begin(); it != bgmVoices_.end();) {
		if (activeBgm.contains(it->first)) { ++it; continue; }
		audio.Stop(it->second);
		it = bgmVoices_.erase(it);
	}

	std::unordered_set<uint32_t> activeSe;
	entities.Each<SCENE::SEComponent>([&](uint32_t id, SCENE::SEComponent& se) {
		activeSe.insert(id);
		if (!se.isPlayReqest) return;
		se.isPlayReqest = false;
		const auto old = loopingSeVoices_.find(id);
		if (old != loopingSeVoices_.end()) {
			audio.Stop(old->second);
			loopingSeVoices_.erase(old);
		}
		const uint32_t handle = audio.Play(se.audioPath, se.isLoop, se.volume, AUDIO::AudioCategory::SE);
		if (handle && se.isLoop) loopingSeVoices_[id] = handle;
		else if (handle) oneShotVoices_.push_back(handle);
	});
	for (auto it = loopingSeVoices_.begin(); it != loopingSeVoices_.end();) {
		if (activeSe.contains(it->first)) { ++it; continue; }
		audio.Stop(it->second);
		it = loopingSeVoices_.erase(it);
	}
	audio.Update();
	// 再生終了した SE のハンドルを残し続けない。
	std::erase_if(oneShotVoices_, [&](uint32_t handle) { return !audio.IsActive(handle); });
}
