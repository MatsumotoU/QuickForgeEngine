#pragma once

#include "audio/AudioEngine.h"

#include <cstdint>
#include <string>

namespace QFE::FRAMEWORK {
	class WindowsQuickForgeEngineSystems;

	// 旧 AudioSourceManager / AudioInterface と同じ二段階の読み込み・再生 API。
	// 0 は無効なデータハンドルまたは再生ハンドルを表す。
	uint32_t LoadSoundData(WindowsQuickForgeEngineSystems& systems, const std::string& filePath);
	uint32_t PlaySoundForAudioData(WindowsQuickForgeEngineSystems& systems,
		uint32_t audioDataHandle, bool loop, float volume,
		AUDIO::AudioCategory category = AUDIO::AudioCategory::SE);
	void StopSound(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle);
	void PauseSound(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle);
	void ResumeSound(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle);
	void StopAllSound(WindowsQuickForgeEngineSystems& systems);
	void PauseAllSound(WindowsQuickForgeEngineSystems& systems);
	void ResumeAllSound(WindowsQuickForgeEngineSystems& systems);
	void SetSoundVolume(WindowsQuickForgeEngineSystems& systems, uint32_t soundHandle, float volume);
	void SetMasterVolume(WindowsQuickForgeEngineSystems& systems, float volume);
	float GetMasterVolume(const WindowsQuickForgeEngineSystems& systems);
	void SetBGMVolume(WindowsQuickForgeEngineSystems& systems, float volume);
	float GetBGMVolume(const WindowsQuickForgeEngineSystems& systems);
	void SetSEVolume(WindowsQuickForgeEngineSystems& systems, float volume);
	float GetSEVolume(const WindowsQuickForgeEngineSystems& systems);
	void SetVoiceVolume(WindowsQuickForgeEngineSystems& systems, float volume);
	float GetVoiceVolume(const WindowsQuickForgeEngineSystems& systems);
	void SetASVolume(WindowsQuickForgeEngineSystems& systems, float volume);
	float GetASVolume(const WindowsQuickForgeEngineSystems& systems);
}
