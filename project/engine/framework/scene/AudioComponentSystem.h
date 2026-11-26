#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace QFE { class EntityManager; }
namespace QFE::AUDIO { class AudioEngine; }
namespace QFE::SCENE { class SceneManager; }

namespace QFE::FRAMEWORK {
	class AudioComponentSystem final {
	public:
		void Update(SCENE::SceneManager& scene, AUDIO::AudioEngine& audio);
		// シーンの ECS を直接受け取り、呼び出し元が更新対象を選べるようにする。
		void Update(EntityManager& entities, uint64_t sceneRevision, AUDIO::AudioEngine& audio);
		void Reset(AUDIO::AudioEngine& audio);
		/// このシーンが所有する音声だけを一時停止・再開する。
		void SetPaused(AUDIO::AudioEngine& audio, bool paused);

	private:
		uint64_t sceneRevision_ = 0;
		std::unordered_map<uint32_t, uint32_t> bgmVoices_;
		std::unordered_map<uint32_t, uint32_t> loopingSeVoices_;
		// ループしない音も、シーンの切り替え時にそのシーンだけ停止する。
		std::vector<uint32_t> oneShotVoices_;
		bool paused_ = false;
	};
}
