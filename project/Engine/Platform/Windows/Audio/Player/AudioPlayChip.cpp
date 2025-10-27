#include "AudioPlayChip.h"
#include <cassert>

AudioPlayChip::~AudioPlayChip() {
	Finalize();
}

void AudioPlayChip::Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice) {
	// 初期化
	volume_ = 1.0f;
	isPaused_ = false;
	isPlaying_ = false;
	sourceVoice_ = nullptr;
	assert(xAudio2 != nullptr);
	assert(masterVoice != nullptr);
	xAudio2_ = xAudio2;
	masterVoice_ = masterVoice;
}

void AudioPlayChip::Finalize() {
	StopSound();
	if (sourceVoice_ != nullptr) {
		sourceVoice_->DestroyVoice();
		sourceVoice_ = nullptr;
	}
}

void AudioPlayChip::PlaySoundForAudioData(AudioData audioData, bool loop, float volume) {
	assert(sourceVoice_ == nullptr && "SourceVoice is already playing");
	assert(xAudio2_ != nullptr);
	assert(masterVoice_ != nullptr);

	// TODO: 音のポストプロセス処理を別の場所に作ること(TD用の緊急オペ)
	// --- チープ化処理ここから ---
	// 8bit化 & サンプリングレート半減
	if (audioData.wfex.wBitsPerSample == 16) {
		// 元データ: 16bit PCM
		const int16_t* src = reinterpret_cast<int16_t*>(audioData.pBuffer);
		size_t sampleCount = audioData.bufferSize / 2;
		// サンプリングレート半減
		size_t cheapSampleCount = sampleCount / 2;
		BYTE* cheapBuffer = new BYTE[cheapSampleCount];
		for (size_t i = 0; i < cheapSampleCount; ++i) {
			// 2サンプルごとに1サンプルだけ使う
			int16_t s = src[i * 2];
			// 8bit化（符号なし）
			cheapBuffer[i] = static_cast<BYTE>((s + 32768) >> 8);
		}
		// WAVEFORMATEXを8bit/サンプリングレート半減に変更
		audioData.wfex.wBitsPerSample = 8;
		audioData.wfex.nBlockAlign = audioData.wfex.nChannels * audioData.wfex.wBitsPerSample / 8;
		audioData.wfex.nAvgBytesPerSec = audioData.wfex.nSamplesPerSec / 2 * audioData.wfex.nBlockAlign;
		audioData.wfex.nSamplesPerSec /= 2;
		audioData.pBuffer = cheapBuffer;
		audioData.bufferSize = static_cast<unsigned int>(cheapSampleCount);
	}
	// --- チープ化処理ここまで ---

	// ソースボイスの作成
	sourceVoice_ = nullptr;
	HRESULT hr = xAudio2_->CreateSourceVoice(&sourceVoice_, &audioData.wfex);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = audioData.bufferSize;
	buffer.pAudioData = audioData.pBuffer;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.LoopBegin = 0;
	buffer.LoopLength = 0;

	hr = sourceVoice_->SubmitSourceBuffer(&buffer);
	assert(SUCCEEDED(hr));

	if (sourceVoice_ != nullptr) {
		sourceVoice_->SetVolume(volume);
		sourceVoice_->Start(0);
		isPlaying_ = true;
		isPaused_ = false;
	}
}

void AudioPlayChip::StopSound() {
	if (sourceVoice_ != nullptr) {
		sourceVoice_->Stop(0);
		sourceVoice_->FlushSourceBuffers();
		sourceVoice_->DestroyVoice();
		sourceVoice_ = nullptr;
		isPlaying_ = false;
		isPaused_ = false;
	}
}

void AudioPlayChip::PauseSound() {
	if (sourceVoice_ != nullptr && isPlaying_ && !isPaused_) {
		sourceVoice_->Stop(0);
		isPaused_ = true;
	}
}

void AudioPlayChip::ResumeSound() {
	if (sourceVoice_ != nullptr && isPlaying_ && isPaused_) {
		sourceVoice_->Start(0);
		isPaused_ = false;
	}
}

void AudioPlayChip::SetVolume(float volume) {
	if (sourceVoice_ != nullptr) {
		volume_ = volume;
		sourceVoice_->SetVolume(volume_);
	}
}

float AudioPlayChip::GetVolume() const {
	if (sourceVoice_ != nullptr) {
		return volume_;
	}
	assert(false && "SourceVoice is nullptr");
	return 0.0f;
}
