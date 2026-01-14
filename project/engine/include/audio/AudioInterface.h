/**
 * @file AudioInterface.h
 * @brief IAudioInterfaceの実装クラス。音声の再生・管理を行う
 */

#pragma once
#include "IAudioInterface.h"
#include "Core/XAudioCore.h"
#include "Player/AudioChipManager.h"
#include "engine/include/utility/DesignPatterns/Singleton.h"

/**
 * @class AudioInterface
 * @brief XAudio2を使用した音声再生機能を制御するシングルトンクラス
 */
class AudioInterface final : public IAudioInterface , public Singleton<AudioInterface>{
	friend class Singleton<AudioInterface>;
public:
	AudioInterface() = default;
	~AudioInterface() override = default;
    
    /** @brief 初期化 */
	void Initialize() override;
    /** @brief 終了処理 */
	void Finalize() override;

	// --- 全体の音声操作 ---
	/** @brief 全ての音声を停止 */
	void StopAllSound() override;
	/** @brief 全ての音声を一時停止 */
	void PauseAllSound() override;
	/** @brief 全ての音声を再開 */
	void ResumeAllSound() override;

	// --- 個別の音声操作 ---
	/**
     * @brief 音声データを再生
     * @param audioDataHandle 再生するデータのハンドル
     * @param loop ループ再生するか
     * @param volume 音量 (0.0 ~ 1.0)
     * @return 再生中の音声インスタンスを示すハンドル
     */
	uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) override;
	/** @brief 指定したハンドル音声を停止 */
	void StopSound(uint32_t soundHandle) override;
	/** @brief 指定したハンドル音声を一時停止 */
	void PauseSound(uint32_t soundHandle) override;
	/** @brief 指定したハンドル音声を再開 */
	void ResumeSound(uint32_t soundHandle) override;

	// --- 各音量設定 (0.0 ~ 1.0) ---
    /** @brief マスター音量を設定 */
	void SetMasterVolume(float volume) override;
	/** @brief BGM音量を設定 */
	void SetBGMVolume(float volume) override;
	/** @brief SE音量を設定 */
	void SetSEVolume(float volume) override;
	/** @brief ボイス音量を設定 */
	void SetVoiceVolume(float volume) override;
	/** @brief 環境音音量を設定 */
	void SetASVolume(float volume) override;

	// --- 各音量取得 ---
	/** @brief マスター音量を取得 */
	float GetMasterVolume() override;
	/** @brief BGM音量を取得 */
	float GetBGMVolume() override;
	/** @brief SE音量を取得 */
	float GetSEVolume() override;
	/** @brief ボイス音量を取得 */
	float GetVoiceVolume() override;
	/** @brief 環境音音量を取得 */
	float GetASVolume() override;

private:
	XAudioCore xAudioCore_;
	AudioChipManager audioChipManager_;
};
