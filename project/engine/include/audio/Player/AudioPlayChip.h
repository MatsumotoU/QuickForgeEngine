#pragma once
#include <xaudio2.h>
#include <memory>
#include <cstdint>
#include "engine/include/assets/AudioSource/Data/AudioData.h"
namespace QFE {
	class AudioPlayChip final {
	public:
		AudioPlayChip() = default;
		~AudioPlayChip();
		// 初期匁E
		void Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice);
		// 終亁E
		void Finalize();
		// 音声再生(volume: 0.0 ~ 1.0の篁E��)
		void PlaySoundForAudioData(const AudioData& audioData, bool loop, float volume);
		// 音声停止
		void StopSound();
		// 音声一時停止
		void PauseSound();
		// 音声再開
		void ResumeSound();
		// 音量設宁E0.0 ~ 1.0の篁E��)
		void SetVolume(float volume);
		// 音量取征E0.0 ~ 1.0の篁E��)
		float GetVolume() const;

	private:
		IXAudio2SourceVoice* sourceVoice_;
		float volume_;
		bool isPaused_;
		bool isPlaying_;
		IXAudio2* xAudio2_;
		IXAudio2MasteringVoice* masterVoice_;
	};
}