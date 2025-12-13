#include "engine/include/audio/Player/AudioPlayChip.h"
#include <cassert>
#include <random>
#include <algorithm>

AudioPlayChip::~AudioPlayChip() {
	Finalize();
}

void AudioPlayChip::Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice) {
	// 蛻晄悄蛹・
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

	// TODO: 髻ｳ縺ｮ繝昴せ繝医・繝ｭ繧ｻ繧ｹ蜃ｦ逅・ｒ蛻･縺ｮ蝣ｴ謇縺ｫ菴懊ｋ縺薙→(TD逕ｨ縺ｮ邱頑･繧ｪ繝・
	// --- 繝√・繝怜喧蜃ｦ逅・％縺薙°繧・---
	// 8bit蛹・& 繧ｵ繝ｳ繝励Μ繝ｳ繧ｰ繝ｬ繝ｼ繝亥濠貂・& 繝弱う繧ｺ & lo-fi繧ｨ繝輔ぉ繧ｯ繝・
	if (audioData.wfex.wBitsPerSample == 16) {
		const int16_t* src = reinterpret_cast<int16_t*>(audioData.pBuffer);
		size_t sampleCount = audioData.bufferSize / 2;
		size_t cheapSampleCount = sampleCount / 2;
		BYTE* cheapBuffer = new BYTE[cheapSampleCount];

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> noiseDist(-32, 32); // 繝弱う繧ｺ
		float lastSample = 0.0f; // 繝ｭ繝ｼ繝代せ繝輔ぅ繝ｫ繧ｿ逕ｨ

		for (size_t i = 0; i < cheapSampleCount; ++i) {
			int16_t s = src[i * 2];

			if (s == 0) {
				cheapBuffer[i] = 128;
				continue;
			}

			// 繝弱う繧ｺ莉伜刈
			s = static_cast<int16_t>(std::clamp<int>(s + noiseDist(gen), -32768, 32767));

			// 豁ｪ縺ｿ・医け繝ｪ繝・ヴ繝ｳ繧ｰ・・
			const int16_t clipLevel = 8000; // lo-fi諢溘ｒ蜃ｺ縺咎明蛟､
			if (s > clipLevel) s = clipLevel;
			if (s < -clipLevel) s = -clipLevel;

			// 繝ｭ繝ｼ繝代せ繝輔ぅ繝ｫ繧ｿ・磯ｫ伜沺繧ｫ繝・ヨ・・
			float alpha = 0.35f; // 繝輔ぅ繝ｫ繧ｿ蠑ｷ蠎ｦ・・.0・・.0・・
			float filtered = lastSample * (1.0f - alpha) + s * alpha;
			lastSample = filtered;
			s = static_cast<int16_t>(filtered);

			// 8bit蛹厄ｼ育ｬｦ蜿ｷ縺ｪ縺暦ｼ・
			cheapBuffer[i] = static_cast<BYTE>((s + 32768) >> 8);
		}
		audioData.wfex.wBitsPerSample = 8;
		audioData.wfex.nBlockAlign = audioData.wfex.nChannels * audioData.wfex.wBitsPerSample / 8;
		audioData.wfex.nAvgBytesPerSec = audioData.wfex.nSamplesPerSec / 2 * audioData.wfex.nBlockAlign;
		audioData.wfex.nSamplesPerSec /= 2;
		audioData.pBuffer = cheapBuffer;
		audioData.bufferSize = static_cast<unsigned int>(cheapSampleCount);
	}
	// --- 繝√・繝怜喧蜃ｦ逅・％縺薙∪縺ｧ ---

	// 繧ｽ繝ｼ繧ｹ繝懊う繧ｹ縺ｮ菴懈・
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
