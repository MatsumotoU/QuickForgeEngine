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

	// TODO: 既存のサンプル・レート変換は別の環境に任せること(TD経由の考慮)
	// --- サンプル・レート変換ここから ---
	// 8bit変換 & サンプリング周波数変更 & ノイズ & lo-fiエフェクト
	if (audioData.wfxEx.Format.wBitsPerSample == 16) {
		const int16_t* src = reinterpret_cast<int16_t*>(audioData.buffer.data());
		size_t sampleCount = audioData.buffer.size() / sizeof(int16_t); // sizeof(int16_t) を追加
		size_t cheapSampleCount = sampleCount / 2;
		std::vector<BYTE> cheapBuffer(cheapSampleCount); // new BYTE[] から std::vector に変更

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> noiseDist(-32, 32); // ノイズ
		float lastSample = 0.0f; // ローパスフィルター用

		for (size_t i = 0; i < cheapSampleCount; ++i) {
			int16_t s = src[i * 2];

			if (s == 0) {
				cheapBuffer[i] = 128;
				continue;
			}

			// ノイズ乗算
			s = static_cast<int16_t>(std::clamp<int>(s + noiseDist(gen), -32768, 32767));

			// 閾値を超えたクリッピング
			const int16_t clipLevel = 8000; // lo-fi感を出す閾値
			if (s > clipLevel) s = clipLevel;
			if (s < -clipLevel) s = -clipLevel;

			// ローパスフィルター・適用
			float alpha = 0.35f; // フィルター強度0.0～1.0
			float filtered = lastSample * (1.0f - alpha) + s * alpha;
			lastSample = filtered;
			s = static_cast<int16_t>(filtered);

			// 8bit変換して格納
			cheapBuffer[i] = static_cast<BYTE>((s + 32768) >> 8);
		}
		audioData.wfxEx.Format.wBitsPerSample = 8;
		audioData.wfxEx.Format.nBlockAlign = audioData.wfxEx.Format.nChannels * audioData.wfxEx.Format.wBitsPerSample / 8;
		audioData.wfxEx.Format.nAvgBytesPerSec = audioData.wfxEx.Format.nSamplesPerSec / 2 * audioData.wfxEx.Format.nBlockAlign;
		audioData.wfxEx.Format.nSamplesPerSec /= 2;
		audioData.buffer = std::move(cheapBuffer); // std::move で所有権を移動
	}
	// --- サンプル・レート変換ここまで ---

	// ソースバッファの作成
	sourceVoice_ = nullptr;
	HRESULT hr = xAudio2_->CreateSourceVoice(&sourceVoice_, &audioData.wfxEx.Format);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = static_cast<UINT32>(audioData.buffer.size()); // bufferSize を buffer.size() に変更
	buffer.pAudioData = audioData.buffer.data(); // pBuffer を buffer.data() に変更
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
