#pragma once
#include <cstdint>

class IAudioInterface {
public:
	virtual ~IAudioInterface() = default;
	virtual void Initialize() = 0;
	virtual void Finalize() = 0;

	// 全体の音声操作
	/// 全音声停止
	virtual void StopAllSound() = 0;
	/// 全音声一時停止
	virtual void PauseAllSound() = 0;
	/// 全音声再開
	virtual void ResumeAllSound() = 0;

	// 個別の音声操作
	/// 音声再生(volume: 0.0 ~ 1.0の範囲)
	virtual uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) = 0;
	/// 音声停止
	virtual void StopSound(uint32_t soundHandle) = 0;
	/// 音声一時停止
	virtual void PauseSound(uint32_t soundHandle) = 0;
	/// 音声再開
	virtual void ResumeSound(uint32_t soundHandle) = 0;

	// 各音量設定
	/// 主音量設定(0.0 ~ 1.0の範囲)
	virtual void SetMasterVolume(float volume) = 0;
	/// BGM音量設定(0.0 ~ 1.0の範囲)
	virtual void SetBGMVolume(float volume) = 0;
	/// SE音量設定(0.0 ~ 1.0の範囲)
	virtual void SetSEVolume(float volume) = 0;
	/// ボイス音量設定(0.0 ~ 1.0の範囲)
	virtual void SetVoiceVolume(float volume) = 0;
	/// AS音量設定(0.0 ~ 1.0の範囲)
	virtual void SetASVolume(float volume) = 0;

	// 各音量取得
	/// 主音量取得(0.0 ~ 1.0の範囲)
	virtual float GetMasterVolume() = 0;
	/// BGM音量取得(0.0 ~ 1.0の範囲)
	virtual float GetBGMVolume() = 0;
	/// SE音量取得(0.0 ~ 1.0の範囲)
	virtual float GetSEVolume() = 0;
	/// ボイス音量取得(0.0 ~ 1.0の範囲)
	virtual float GetVoiceVolume() = 0;
	/// AS音量取得(0.0 ~ 1.0の範囲)
	virtual float GetASVolume() = 0;
};