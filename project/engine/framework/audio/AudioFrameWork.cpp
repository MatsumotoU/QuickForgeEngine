#include "AudioFrameWork.h"
#include "framework/application/WindowsQuickForgeEngineSystems.h"

uint32_t QFE::FRAMEWORK::LoadSoundData(WindowsQuickForgeEngineSystems& systems, const std::string& filePath) {
	return systems.audioEngine ? systems.audioEngine->LoadSoundData(filePath) : 0;
}

uint32_t QFE::FRAMEWORK::PlaySoundForAudioData(WindowsQuickForgeEngineSystems& systems,
	uint32_t audioDataHandle, bool loop, float volume, AUDIO::AudioCategory category) {
	return systems.audioEngine
		? systems.audioEngine->PlaySoundForAudioData(audioDataHandle, loop, volume, category) : 0;
}

void QFE::FRAMEWORK::StopSound(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle) {
	if (systems.audioEngine) systems.audioEngine->Stop(soundHandle);
}
void QFE::FRAMEWORK::PauseSound(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle) {
	if (systems.audioEngine) systems.audioEngine->Pause(soundHandle);
}
void QFE::FRAMEWORK::ResumeSound(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle) {
	if (systems.audioEngine) systems.audioEngine->Resume(soundHandle);
}
void QFE::FRAMEWORK::StopAllSound(WindowsQuickForgeEngineSystems& systems) {
	if (systems.audioEngine) systems.audioEngine->StopAll();
}
void QFE::FRAMEWORK::PauseAllSound(WindowsQuickForgeEngineSystems& systems) {
	if (systems.audioEngine) systems.audioEngine->PauseAll();
}
void QFE::FRAMEWORK::ResumeAllSound(WindowsQuickForgeEngineSystems& systems) {
	if (systems.audioEngine) systems.audioEngine->ResumeAll();
}
void QFE::FRAMEWORK::SetSoundVolume(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle, float volume) {
	if (systems.audioEngine) systems.audioEngine->SetVolume(soundHandle, volume);
}
void QFE::FRAMEWORK::SetMasterVolume(WindowsQuickForgeEngineSystems& systems, float volume) {
	if (systems.audioEngine) systems.audioEngine->SetMasterVolume(volume);
}
float QFE::FRAMEWORK::GetMasterVolume(const WindowsQuickForgeEngineSystems& systems) {
	return systems.audioEngine ? systems.audioEngine->GetMasterVolume() : 0.0f;
}
void QFE::FRAMEWORK::SetBGMVolume(WindowsQuickForgeEngineSystems& systems, float volume) {
	if (systems.audioEngine) systems.audioEngine->SetCategoryVolume(AUDIO::AudioCategory::BGM, volume);
}
float QFE::FRAMEWORK::GetBGMVolume(const WindowsQuickForgeEngineSystems& systems) {
	return systems.audioEngine ? systems.audioEngine->GetCategoryVolume(AUDIO::AudioCategory::BGM) : 0.0f;
}
void QFE::FRAMEWORK::SetSEVolume(WindowsQuickForgeEngineSystems& systems, float volume) {
	if (systems.audioEngine) systems.audioEngine->SetCategoryVolume(AUDIO::AudioCategory::SE, volume);
}
float QFE::FRAMEWORK::GetSEVolume(const WindowsQuickForgeEngineSystems& systems) {
	return systems.audioEngine ? systems.audioEngine->GetCategoryVolume(AUDIO::AudioCategory::SE) : 0.0f;
}
void QFE::FRAMEWORK::SetVoiceVolume(WindowsQuickForgeEngineSystems& systems, float volume) {
	if (systems.audioEngine) systems.audioEngine->SetCategoryVolume(AUDIO::AudioCategory::Voice, volume);
}
float QFE::FRAMEWORK::GetVoiceVolume(const WindowsQuickForgeEngineSystems& systems) {
	return systems.audioEngine ? systems.audioEngine->GetCategoryVolume(AUDIO::AudioCategory::Voice) : 0.0f;
}
void QFE::FRAMEWORK::SetASVolume(WindowsQuickForgeEngineSystems& systems, float volume) {
	if (systems.audioEngine) systems.audioEngine->SetCategoryVolume(AUDIO::AudioCategory::Ambient, volume);
}
float QFE::FRAMEWORK::GetASVolume(const WindowsQuickForgeEngineSystems& systems) {
	return systems.audioEngine ? systems.audioEngine->GetCategoryVolume(AUDIO::AudioCategory::Ambient) : 0.0f;
}
