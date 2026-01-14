/**
 * @file IAudioInterface.h
 * @brief オーディオインターフェースの基底クラス
 */

#pragma once
#include <cstdint>

/**
 * @class IAudioInterface
 * @brief 音声再生・音量管理の抽象インターフェース。XAudio2等の各実装が継承する
 */
class IAudioInterface {
public:
	virtual ~IAudioInterface() = default;
    /** @brief 初期化 */
	virtual void Initialize() = 0;
    /** @brief 終了処理 */
	virtual void Finalize() = 0;

	// --- 全体の音声操作 ---
	/** @brief 全音声停止 */
	virtual void StopAllSound() = 0;
	/** @brief 全音声一時停止 */
	virtual void PauseAllSound() = 0;
	/** @brief 全音声再開 */
	virtual void ResumeAllSound() = 0;

	// --- 個別の音声操作 ---
	/**
     * @brief 音声を再生
     * @param audioDataHandle 音声データハンドル
     * @param loop ループ再生フラグ
     * @param volume 音量 (0.0 ~ 1.0)
     * @return サウンドハンドル
     */
	virtual uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) = 0;
	/** @brief 音声停止 */
	virtual void StopSound(uint32_t soundHandle) = 0;
	/** @brief 音声一時停止 */
	virtual void PauseSound(uint32_t soundHandle) = 0;
	/** @brief 音声再開 */
	virtual void ResumeSound(uint32_t soundHandle) = 0;

	// --- 音量設定 (0.0 ~ 1.0) ---
	/** @brief マスター音量を設定 */
	virtual void SetMasterVolume(float volume) = 0;
	/** @brief BGM音量を設定 */
	virtual void SetBGMVolume(float volume) = 0;
	/** @brief SE音量を設定 */
	virtual void SetSEVolume(float volume) = 0;
	/** @brief ボイス音量を設定 */
	virtual void SetVoiceVolume(float volume) = 0;
	/** @brief 環境音音量を設定 */
	virtual void SetASVolume(float volume) = 0;

	// --- 音量取得 ---
	/** @brief マスター音量を取得 */
	virtual float GetMasterVolume() = 0;
	/** @brief BGM音量を取得 */
	virtual float GetBGMVolume() = 0;
	/** @brief SE音量を取得 */
	virtual float GetSEVolume() = 0;
	/** @brief ボイス音量を取得 */
	virtual float GetVoiceVolume() = 0;
	/** @brief 環境音音量を取得 */
	virtual float GetASVolume() = 0;
};
