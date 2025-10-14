#pragma once
#include <xaudio2.h>
#include <unordered_map>
#include "AudioPlayChip.h"

class AudioChipManager final {
public:
	AudioChipManager() = default;
	~AudioChipManager();
	// 初期化
	void Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice);
	// 終了
	void Finalize();
	// 音声再生(volume: 0.0 ~ 1.0の範囲)
	uint32_t PlaySoundForAudioData(AudioData audioData, bool loop, float volume);
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
	// 音量設定(0.0 ~ 1.0の範囲)
	void SetVolume(uint32_t soundHandle, float volume);
	// 音量取得(0.0 ~ 1.0の範囲)
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