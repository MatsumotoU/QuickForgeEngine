#include "Audio3D.h"
#include <vector>
#include <cassert>

#include "AudioManager.h"

void Audio3D::Initialize(IXAudio2MasteringVoice* pMasterVoice) {
	// X3DAudioの初期化
	DWORD dwChannelMask;
	pMasterVoice->GetChannelMask(&dwChannelMask);
	HRESULT hr = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, x3DInstance_);
	assert(SUCCEEDED(hr));
	
}

X3DAUDIO_DSP_SETTINGS Audio3D::CreateDspSettings(X3DAUDIO_LISTENER* listener,X3DAUDIO_EMITTER* emitter, const SoundData& soundData) {
	X3DAUDIO_DSP_SETTINGS dspSettings = { 0 };
	dspSettings.SrcChannelCount = emitter->ChannelCount;
	dspSettings.DstChannelCount = emitter->ChannelCount;
	std::vector<float> matrix(soundData.wfex.nChannels * 2);
	dspSettings.pMatrixCoefficients = matrix.data();
	std::vector<float> delayTimes(emitter->ChannelCount * emitter->ChannelCount, 0.0f); // 例: 全て 0.0f で初期化
	dspSettings.pDelayTimes = delayTimes.data();

	X3DAudioCalculate(
		x3DInstance_,
		listener,
		emitter,
		X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT,
		&dspSettings
	);

	return dspSettings;
}

X3DAUDIO_HANDLE* Audio3D::GetX3DInstance() {
	return &x3DInstance_;
}

IXAudio2SourceVoice* audio3d::Create3DSourceVoice(IXAudio2* xAudio2, IXAudio2MasteringVoice* pMasterVoice, const SoundData& soundData, const X3DAUDIO_DSP_SETTINGS& dspSettings) {
	// ソースボイス作成
	IXAudio2SourceVoice* pSourceVoice = Audiomanager::CreateSourceVoice(xAudio2, soundData);
	HRESULT hr{};

	//XAUDIO2_EFFECTS_SENDS effectSendDesc = { 1, { &reverbSend } }; // リバーブサブミックスボイスへの送信 (ある場合)

	hr = pSourceVoice->SetOutputMatrix(pMasterVoice, soundData.wfex.nChannels, 2, dspSettings.pMatrixCoefficients);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->SetFrequencyRatio(XAUDIO2_DEFAULT_FREQ_RATIO * dspSettings.DopplerFactor);
	assert(SUCCEEDED(hr));

	// ローパスフィルターの設定
	/*XAUDIO2_FILTER_PARAMETERS filterParams = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient), 1.0f };
	hr = pSourceVoice->SetFilterParameters(&filterParams);
	assert(SUCCEEDED(hr));*/

	// リバーブ送信レベルの設定 (サブミックスボイスを使用する場合)
	// pSourceVoice->SetEffectSendLevel(0, dspSettings.ReverbLevel);

	return pSourceVoice;
}