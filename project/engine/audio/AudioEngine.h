#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace QFE::AUDIO {
	enum class AudioCategory { BGM, SE, Voice, Ambient };

	// Media Foundation で音声を PCM に読み込み、XAudio2 で再生する。
	class AudioEngine final {
	public:
		AudioEngine();
		~AudioEngine();
		AudioEngine(const AudioEngine&) = delete;
		AudioEngine& operator=(const AudioEngine&) = delete;

		bool Initialize();
		void Shutdown();
		uint32_t LoadSoundData(const std::string& audioPath);
		uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume,
			AudioCategory category = AudioCategory::SE);
		uint32_t Play(const std::string& audioPath, bool loop, float volume = 1.0f,
			AudioCategory category = AudioCategory::SE);
		void Stop(uint32_t handle);
		void Pause(uint32_t handle);
		void Resume(uint32_t handle);
		void StopAll();
		void PauseAll();
		void ResumeAll();
		void SetMasterVolume(float volume);
		float GetMasterVolume() const;
		void SetCategoryVolume(AudioCategory category, float volume);
		float GetCategoryVolume(AudioCategory category) const;
		void SetVolume(uint32_t handle, float volume);
		void Update();

	private:
		struct Impl;
		std::unique_ptr<Impl> impl_;
	};
}
