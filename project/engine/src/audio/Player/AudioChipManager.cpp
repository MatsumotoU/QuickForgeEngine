#include "engine/include/audio/Player/AudioChipManager.h"
#include "engine/include/core/EngineDefines.h"

using namespace QFE;
AudioChipManager::~AudioChipManager() {
	Finalize();
}

void AudioChipManager::Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice) {
	xAudio2_ = xAudio2;
	masterVoice_ = masterVoice;
	nextHandle_ = 1; // エラーハンドルを避けるため1から開姁E
	audioChips_.clear();
}

void AudioChipManager::Finalize() {
	StopAllSound();
	audioChips_.clear();
}

uint32_t AudioChipManager::PlaySoundForAudioData(const AudioData& audioData, bool loop, float volume) {
	audioChips_.emplace(nextHandle_, AudioPlayChip());
	audioChips_[nextHandle_].Initialize(xAudio2_, masterVoice_);
	audioChips_[nextHandle_].PlaySoundForAudioData(audioData, loop, volume);
	return nextHandle_++;
}

void AudioChipManager::StopSound(uint32_t soundHandle) {
	try {
		if (audioChips_.find(soundHandle) != audioChips_.end()) {
			audioChips_[soundHandle].StopSound();
			audioChips_[soundHandle].Finalize();
			audioChips_.erase(soundHandle);
		}
	}
	catch (const std::exception& e) {
		e;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::string("StopSound error: ") + e.what());
#endif // QFE_OPTIMIZE_OFF
	}
}

void AudioChipManager::PauseSound(uint32_t soundHandle) {
	try {
		if (audioChips_.find(soundHandle) != audioChips_.end()) {
			audioChips_[soundHandle].PauseSound();
		}
	}
	catch (const std::exception& e) {
		e;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::string("PauseSound error: ") + e.what());
#endif // QFE_OPTIMIZE_OFF
	}
}

void AudioChipManager::ResumeSound(uint32_t soundHandle) {
	try {
		if (audioChips_.find(soundHandle) != audioChips_.end()) {
			audioChips_[soundHandle].ResumeSound();
		}
	}
	catch (const std::exception& e) {
		e;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::string("ResumeSound error: ") + e.what());
#endif // QFE_OPTIMIZE_OFF
	}
}

void AudioChipManager::StopAllSound() {
	for (auto& [handle, chip] : audioChips_) {
		chip.StopSound();
		chip.Finalize();
	}
	audioChips_.clear();
}

void AudioChipManager::PauseAllSound() {
	for (auto& [handle, chip] : audioChips_) {
		chip.PauseSound();
	}
}

void AudioChipManager::ResumeAllSound() {
	for (auto& [handle, chip] : audioChips_) {
		chip.ResumeSound();
	}
}

void AudioChipManager::SetVolume(uint32_t soundHandle, float volume) {
	try {
		if (audioChips_.find(soundHandle) != audioChips_.end()) {
			audioChips_[soundHandle].SetVolume(volume);
		}
	}
	catch (const std::exception& e) {
		e;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::string("SetVolume error: ") + e.what());
#endif // QFE_OPTIMIZE_OFF
	}
}

float AudioChipManager::GetVolume(uint32_t soundHandle) const {
	try {
		if (audioChips_.find(soundHandle) != audioChips_.end()) {
			return audioChips_.at(soundHandle).GetVolume();
		}
	}
	catch (const std::exception& e) {
		e;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::string("GetVolume error: ") + e.what());
#endif // QFE_OPTIMIZE_OFF
	}
	return 0.0f;
}
