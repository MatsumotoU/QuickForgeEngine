#pragma once
#include <xaudio2.h>
#include <unordered_map>
#include "AudioPlayChip.h"

class AudioChipManager final {
public:
	AudioChipManager() = default;
	~AudioChipManager();
	// 初期匁E
	void Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice);
	// 終亁E
	void Finalize();
	// 音声再生(volume: 0.0 ~ 1.0の篁E��)
	uint32_t PlaySoundForAudioData(const AudioData& audioData, bool loop, float volume);
	// 音声停止
	void StopSound(uint32_t soundHandle);
	// 音声一時停止
	void PauseSound(uint32_t soundHandle);
	// 音声再開
	void ResumeSound(uint32_t soundHandle);
	// 全音声停止
	void StopAllSound();
	// 全音声一時停止
	void PauseAllSound();
	// 全音声再開
	void ResumeAllSound();
	// 音量設宁E0.0 ~ 1.0の篁E��)
	void SetVolume(uint32_t soundHandle, float volume);
	// 音量取征E0.0 ~ 1.0の篁E��)
	float GetVolume(uint32_t soundHandle) const;
private:
	std::unordered_map<uint32_t, AudioPlayChip> audioChips_;
	uint32_t nextHandle_;
	IXAudio2* xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;

	float bgmVolume_;
	float seVolume_;
	float voiceVolume_;
	float asVolume_;
};
