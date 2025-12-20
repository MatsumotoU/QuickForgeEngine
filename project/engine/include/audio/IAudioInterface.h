#pragma once
#include <cstdint>

class IAudioInterface {
public:
	virtual ~IAudioInterface() = default;
	virtual void Initialize() = 0;
	virtual void Finalize() = 0;

	// 全体�E音声操佁E
	/// 全音声停止
	virtual void StopAllSound() = 0;
	/// 全音声一時停止
	virtual void PauseAllSound() = 0;
	/// 全音声再開
	virtual void ResumeAllSound() = 0;

	// 個別の音声操佁E
	/// 音声再生(volume: 0.0 ~ 1.0の篁E��)
	virtual uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) = 0;
	/// 音声停止
	virtual void StopSound(uint32_t soundHandle) = 0;
	/// 音声一時停止
	virtual void PauseSound(uint32_t soundHandle) = 0;
	/// 音声再開
	virtual void ResumeSound(uint32_t soundHandle) = 0;

	// 吁E��量設宁E
	/// 主音量設宁E0.0 ~ 1.0の篁E��)
	virtual void SetMasterVolume(float volume) = 0;
	/// BGM音量設宁E0.0 ~ 1.0の篁E��)
	virtual void SetBGMVolume(float volume) = 0;
	/// SE音量設宁E0.0 ~ 1.0の篁E��)
	virtual void SetSEVolume(float volume) = 0;
	/// ボイス音量設宁E0.0 ~ 1.0の篁E��)
	virtual void SetVoiceVolume(float volume) = 0;
	/// AS音量設宁E0.0 ~ 1.0の篁E��)
	virtual void SetASVolume(float volume) = 0;

	// 吁E��量取征E
	/// 主音量取征E0.0 ~ 1.0の篁E��)
	virtual float GetMasterVolume() = 0;
	/// BGM音量取征E0.0 ~ 1.0の篁E��)
	virtual float GetBGMVolume() = 0;
	/// SE音量取征E0.0 ~ 1.0の篁E��)
	virtual float GetSEVolume() = 0;
	/// ボイス音量取征E0.0 ~ 1.0の篁E��)
	virtual float GetVoiceVolume() = 0;
	/// AS音量取征E0.0 ~ 1.0の篁E��)
	virtual float GetASVolume() = 0;
};
