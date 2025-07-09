#include "Audio3D.h"
#include <cassert>

#include "XAudioCore.h"

void Audio3D::Initialize(XAudioCore* xAudioCore) {
	// X3DAudioの初期化
	DWORD dwChannelMask;
	xAudioCore->GetMasterVoice()->GetChannelMask(&dwChannelMask);
	HRESULT hr = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, x3DInstance_);
	assert(SUCCEEDED(hr));
	hr;

	audioManager_ = xAudioCore;
	assert(xAudioCore);
}

X3DAUDIO_DSP_SETTINGS Audio3D::CreateDspSettings(X3DAUDIO_LISTENER* listener, X3DAUDIO_EMITTER* emitter, std::vector<float>& matrix, std::vector<float>& delayTimes) {
	X3DAUDIO_DSP_SETTINGS dspSettings = { 0 };
	dspSettings.SrcChannelCount = emitter->ChannelCount;
	dspSettings.DstChannelCount = audioManager_->GetOutputChannels();
	dspSettings.pMatrixCoefficients = matrix.data();
	dspSettings.pDelayTimes = delayTimes.data();
	dspSettings.DopplerFactor = 1.0f;

	X3DAudioCalculate(
		x3DInstance_,
		listener,
		emitter,
		X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT,
		&dspSettings
	);

	return dspSettings;
}

std::vector<float> Audio3D::GetMatrixCoefficients(const SoundData& soundData) {
	std::vector<float> matrix(soundData.wfex.nChannels * audioManager_->GetOutputChannels());
	return matrix;
}

std::vector<float> Audio3D::GetDelayTimes(const SoundData& soundData) {
	std::vector<float> delayTimes(soundData.wfex.nChannels * audioManager_->GetOutputChannels(), 0.0f);
	return delayTimes;
}

X3DAUDIO_HANDLE* Audio3D::GetX3DInstance() {
	return &x3DInstance_;
}

IXAudio2SourceVoice* audio3d::Create3DSourceVoice(XAudioCore* audioManager, const SoundData& soundData, const X3DAUDIO_DSP_SETTINGS& dspSettings) {
	// ソースボイス作成
	IXAudio2SourceVoice* pSourceVoice = Audiomanager::CreateSourceVoice(audioManager->xAudio2_.Get(), soundData);
	HRESULT hr{};

	//XAUDIO2_EFFECTS_SENDS effectSendDesc = { 1, { &reverbSend } };

	hr = pSourceVoice->SetOutputMatrix(audioManager->GetMasterVoice(), soundData.wfex.nChannels, audioManager->GetOutputChannels(), dspSettings.pMatrixCoefficients);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->SetFrequencyRatio(XAUDIO2_DEFAULT_FREQ_RATIO * dspSettings.DopplerFactor);
	assert(SUCCEEDED(hr));

	// ローパスフィルターの設定
	XAUDIO2_FILTER_PARAMETERS filterParams = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient), 1.0f };
	hr = pSourceVoice->SetFilterParameters(&filterParams);
	assert(SUCCEEDED(hr));

	// リバーブ送信レベルの設定 (サブミックスボイスを使用する場合)
	// pSourceVoice->SetEffectSendLevel(0, dspSettings.ReverbLevel);

	return pSourceVoice;
}
