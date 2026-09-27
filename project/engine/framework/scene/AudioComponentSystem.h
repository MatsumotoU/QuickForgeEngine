#pragma once

#include <cstdint>
#include <unordered_map>

namespace QFE::AUDIO { class AudioEngine; }
namespace QFE::SCENE { class SceneManager; }

namespace QFE::FRAMEWORK {
	class AudioComponentSystem final {
	public:
		void Update(SCENE::SceneManager& scene, AUDIO::AudioEngine& audio);
		void Reset(AUDIO::AudioEngine& audio);

	private:
		uint64_t sceneRevision_ = 0;
		std::unordered_map<uint32_t, uint32_t> bgmVoices_;
		std::unordered_map<uint32_t, uint32_t> loopingSeVoices_;
	};
}
