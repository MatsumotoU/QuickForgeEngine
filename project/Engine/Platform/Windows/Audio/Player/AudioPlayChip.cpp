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
	sourceVoice_ = nullptr;
}

void AudioPlayChip::PlaySoundForAudioData(AudioData audioData, bool loop, float volume) {
	assert(sourceVoice_ == nullptr && "SourceVoice is already playing");
	assert(xAudio2_ != nullptr);
	assert(masterVoice_ != nullptr);
	// ソースボイスの作成
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
