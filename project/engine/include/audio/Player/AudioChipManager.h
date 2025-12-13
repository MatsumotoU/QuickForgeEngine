#pragma once
#include <xaudio2.h>
#include <unordered_map>
#include "AudioPlayChip.h"

class AudioChipManager final {
public:
	AudioChipManager() = default;
	~AudioChipManager();
	// 蛻晄悄蛹・
	void Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice);
	// 邨ゆｺ・
	void Finalize();
	// 髻ｳ螢ｰ蜀咲函(volume: 0.0 ~ 1.0縺ｮ遽・峇)
	uint32_t PlaySoundForAudioData(AudioData audioData, bool loop, float volume);
	// 髻ｳ螢ｰ蛛懈ｭ｢
	void StopSound(uint32_t soundHandle);
	// 髻ｳ螢ｰ荳譎ょ●豁｢
	void PauseSound(uint32_t soundHandle);
	// 髻ｳ螢ｰ蜀埼幕
	void ResumeSound(uint32_t soundHandle);
	// 蜈ｨ髻ｳ螢ｰ蛛懈ｭ｢
	void StopAllSound();
	// 蜈ｨ髻ｳ螢ｰ荳譎ょ●豁｢
	void PauseAllSound();
	// 蜈ｨ髻ｳ螢ｰ蜀埼幕
	void ResumeAllSound();
	// 髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	void SetVolume(uint32_t soundHandle, float volume);
	// 髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
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
