#pragma once
#include "IAudioInterface.h"
#include "Core/XAudioCore.h"
#include "Player/AudioChipManager.h"
#include "Utility/DesignPatterns/Singleton.h"

class AudioInterface final : public IAudioInterface , public Singleton<AudioInterface>{
	friend class Singleton<AudioInterface>;
public:
	AudioInterface() = default;
	~AudioInterface() override = default;
	void Initialize() override;
	void Finalize() override;

	// 全体の音声操作
	/// 全音声停止
	void StopAllSound() override;
	/// 全音声一時停止
	void PauseAllSound() override;
	/// 全音声再開
	void ResumeAllSound() override;

	// 個別の音声操作
	/// 音声再生(volume: 0.0 ~ 1.0の範囲)
	uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) override;
	/// 音声停止
	void StopSound(uint32_t soundHandle) override;
	/// 音声一時停止
	void PauseSound(uint32_t soundHandle) override;
	/// 音声再開
	void ResumeSound(uint32_t soundHandle) override;

	// 各音量設定
	/// 主音量設定(0.0 ~ 1.0の範囲)
	void SetMasterVolume(float volume) override;
	/// BGM音量設定(0.0 ~ 1.0の範囲)
	void SetBGMVolume(float volume) override;
	/// SE音量設定(0.0 ~ 1.0の範囲)
	void SetSEVolume(float volume) override;
	/// ボイス音量設定(0.0 ~ 1.0の範囲)
	void SetVoiceVolume(float volume) override;
	/// 環境音量設定(0.0 ~ 1.0の範囲)
	void SetASVolume(float volume) override;

	// 各音量取得
	/// 主音量取得(0.0 ~ 1.0の範囲)
	float GetMasterVolume() override;
	/// BGM音量取得(0.0 ~ 1.0の範囲)
	float GetBGMVolume() override;
	/// SE音量取得(0.0 ~ 1.0の範囲)
	float GetSEVolume() override;
	/// ボイス音量取得(0.0 ~ 1.0の範囲)
	float GetVoiceVolume() override;
	/// 環境音量取得(0.0 ~ 1.0の範囲)
	float GetASVolume() override;

private:
	XAudioCore xAudioCore_;
	AudioChipManager audioChipManager_;
};
