#include "engine/include/audio/AudioInterface.h"
#include "engine/include/assets/AssetManager.h"
#include <cassert>

void AudioInterface::Initialize() {
	xAudioCore_.Initialize();
	audioChipManager_.Initialize(xAudioCore_.GetXAudio2(), xAudioCore_.GetMasterVoice());
}

void AudioInterface::Finalize() {
	audioChipManager_.Finalize();
	xAudioCore_.Finalize();
}

void AudioInterface::StopAllSound() {
	audioChipManager_.StopAllSound();
}

void AudioInterface::PauseAllSound() {
	audioChipManager_.PauseAllSound();
}

void AudioInterface::ResumeAllSound() {
	audioChipManager_.ResumeAllSound();
}

uint32_t AudioInterface::PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) {
	const AudioData& audioData = AssetManager::GetInstance()->GetAudioSourceManager()->GetSoundData(audioDataHandle);
	if (audioData.buffer.empty()) { // pBuffer から buffer.empty() に変更
		assert(false && "Audio data is empty"); // アサートメッセージも修正
		return 0;
	}
	return audioChipManager_.PlaySoundForAudioData(audioData, loop, volume);
}

void AudioInterface::StopSound(uint32_t soundHandle) {
	audioChipManager_.StopSound(soundHandle);
}

void AudioInterface::PauseSound(uint32_t soundHandle) {
	audioChipManager_.PauseSound(soundHandle);
}

void AudioInterface::ResumeSound(uint32_t soundHandle) {
	audioChipManager_.ResumeSound(soundHandle);
}

void AudioInterface::SetMasterVolume(float volume) {
	xAudioCore_.SetMasterVolume(volume);
}

void AudioInterface::SetBGMVolume(float volume) {
	volume;
}

void AudioInterface::SetSEVolume(float volume) {
	volume;
}

void AudioInterface::SetVoiceVolume(float volume) {
	volume;
}

void AudioInterface::SetASVolume(float volume) {
	volume;
}

float AudioInterface::GetMasterVolume() {
	return 0.0f;
}

float AudioInterface::GetBGMVolume() {
	return 0.0f;
}

float AudioInterface::GetSEVolume() {
	return 0.0f;
}

float AudioInterface::GetVoiceVolume() {
	return 0.0f;
}

float AudioInterface::GetASVolume() {
	return 0.0f;
}
