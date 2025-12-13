#pragma once
#include <xaudio2.h>
#include <memory>
#include <cstdint>
#include "engine/include/assets/AudioSource/Data/AudioData.h"

class AudioPlayChip final {
public:
	AudioPlayChip() = default;
	~AudioPlayChip();
	// 蛻晄悄蛹・
	void Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice);
	// 邨ゆｺ・
	void Finalize();
	// 髻ｳ螢ｰ蜀咲函(volume: 0.0 ~ 1.0縺ｮ遽・峇)
	void PlaySoundForAudioData(AudioData audioData, bool loop, float volume);
	// 髻ｳ螢ｰ蛛懈ｭ｢
	void StopSound();
	// 髻ｳ螢ｰ荳譎ょ●豁｢
	void PauseSound();
	// 髻ｳ螢ｰ蜀埼幕
	void ResumeSound();
	// 髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	void SetVolume(float volume);
	// 髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	float GetVolume() const;

private:
	IXAudio2SourceVoice* sourceVoice_;
	float volume_;
	bool isPaused_;
	bool isPlaying_;
	IXAudio2* xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;
};
