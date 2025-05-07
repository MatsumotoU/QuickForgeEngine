#pragma once
#include <vector>

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <x3daudio.h>
#include "SoundData.h"

class AudioManager;

class Audio3D {

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