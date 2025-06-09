#pragma once
#include <vector>

#include <xaudio2.h>
#include <x3daudio.h>
#include "SoundData.h"

class AudioManager;

class Audio3D final {

public:
	void Initialize(AudioManager* audioManager);

public:
	X3DAUDIO_DSP_SETTINGS CreateDspSettings(
		X3DAUDIO_LISTENER* listener,X3DAUDIO_EMITTER* emitter,
		std::vector<float>& matrix, std::vector<float>& delayTimes);

public:
	/// <summary>
	/// サウンドデータから行列を作成します
	/// </summary>
	/// <param name="soundData"></param>
	/// <returns></returns>
	std::vector<float> GetMatrixCoefficients(const SoundData& soundData);
	/// <summary>
	/// サウンドデータからディレイタイムを作成します
	/// </summary>
	/// <param name="soundData"></param>
	/// <returns></returns>
	std::vector<float> GetDelayTimes(const SoundData& soundData);
	X3DAUDIO_HANDLE* GetX3DInstance();

private:
	AudioManager* audioManager_;
	X3DAUDIO_HANDLE x3DInstance_;
};

namespace audio3d {

	IXAudio2SourceVoice* Create3DSourceVoice(AudioManager* audioManager, const SoundData& soundData,const X3DAUDIO_DSP_SETTINGS& dspSettings);
}

// 立体音響作った時のコード
/*AudioEmitter emitter{};
	emitter.position_.z = 0.0f;
	emitter.position_.x = 0.0f;
	emitter.nChannels_ = static_cast<uint32_t>(soundData3.wfex.nChannels);
	AudioListener listener{};
	listener.position_.z = -5.0f;
	std::vector<float> matrix = audio3D->GetMatrixCoefficients(soundData3);
	std::vector<float> delayTimes = audio3D->GetDelayTimes(soundData3);
	X3DAUDIO_DSP_SETTINGS settings = audio3D->CreateDspSettings(listener.GetListener(), emitter.GetEmitter(), matrix,delayTimes);
	IXAudio2SourceVoice* sourceVoice = audio3d::Create3DSourceVoice(&audioManager, soundData3, settings);*/