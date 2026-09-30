#include "AudioComponentSystem.h"

#include "audio/AudioEngine.h"
#include "components/BGMComponent.h"
#include "components/SEComponent.h"
#include "scene/SceneManager.h"

#include <unordered_set>

void QFE::FRAMEWORK::AudioComponentSystem::Reset(AUDIO::AudioEngine& audio) {
	audio.StopAll();
	bgmVoices_.clear();
	loopingSeVoices_.clear();
	sceneRevision_ = 0;
}

void QFE::FRAMEWORK::AudioComponentSystem::Update(SCENE::SceneManager& scene, AUDIO::AudioEngine& audio) {
	const bool sceneStarted = sceneRevision_ != scene.GetSceneRevision();
	if (sceneStarted) {
		audio.StopAll();
		bgmVoices_.clear();
		loopingSeVoices_.clear();
		sceneRevision_ = scene.GetSceneRevision();
	}

	auto& entities = scene.GetCurrentSceneEntityManager();
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
	});
	for (auto it = loopingSeVoices_.begin(); it != loopingSeVoices_.end();) {
		if (activeSe.contains(it->first)) { ++it; continue; }
		audio.Stop(it->second);
		it = loopingSeVoices_.erase(it);
	}
	audio.Update();
}
