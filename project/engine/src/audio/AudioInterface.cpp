#include "engine/include/audio/AudioInterface.h"
#include "engine/include/assets/AssetManager.h"
#include <cassert>

namespace QFE {

	/** @brief 初期化 */
	void AudioInterface::Initialize() {
		xAudioCore_.Initialize();
		audioChipManager_.Initialize(xAudioCore_.GetXAudio2(), xAudioCore_.GetMasterVoice());
	}

	/** @brief 終了処理 */
	void AudioInterface::Finalize() {
		audioChipManager_.Finalize();
		xAudioCore_.Finalize();
	}

	/** @brief すべての音を停止 */
	void AudioInterface::StopAllSound() {
		audioChipManager_.StopAllSound();
	}

	/** @brief すべての音を一時停止 */
	void AudioInterface::PauseAllSound() {
		audioChipManager_.PauseAllSound();
	}

	/** @brief すべての音を再開 */
	void AudioInterface::ResumeAllSound() {
		audioChipManager_.ResumeAllSound();
	}

	/**
	 * @brief アセットハンドルを指定して再生
	 * @param audioDataHandle 再生する音声データのハンドル
	 * @param loop ループ再生するか
	 * @param volume 音量
	 * @return サウンド再生ハンドル
	 */
	uint32_t AudioInterface::PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) {
		const AudioData& audioData = AssetManager::GetInstance()->GetAudioSourceManager()->GetSoundData(audioDataHandle);
		if (audioData.buffer.empty()) { // pBuffer から buffer.empty() に変更
			assert(false && "Audio data is empty"); // アサートメッセージも修正
			return 0;
		}
		return audioChipManager_.PlaySoundForAudioData(audioData, loop, volume);
	}

	/** @brief 音を停止 */
	void AudioInterface::StopSound(uint32_t soundHandle) {
		audioChipManager_.StopSound(soundHandle);
	}

	/** @brief 音を一時停止 */
	void AudioInterface::PauseSound(uint32_t soundHandle) {
		audioChipManager_.PauseSound(soundHandle);
	}

	/** @brief 音を再開 */
	void AudioInterface::ResumeSound(uint32_t soundHandle) {
		audioChipManager_.ResumeSound(soundHandle);
	}

	/** @brief マスターボリュームの設定 */
	void AudioInterface::SetMasterVolume(float volume) {
		xAudioCore_.SetMasterVolume(volume);
	}

	/** @brief BGMボリュームの設定 (TODO: 未実装) */
	void AudioInterface::SetBGMVolume(float volume) {
		volume;
	}

	/** @brief SEボリュームの設定 (TODO: 未実装) */
	void AudioInterface::SetSEVolume(float volume) {
		volume;
	}

	/** @brief ボイスボリュームの設定 (TODO: 未実装) */
	void AudioInterface::SetVoiceVolume(float volume) {
		volume;
	}

	/** @brief ASボリュームの設定 (TODO: 未実装) */
	void AudioInterface::SetASVolume(float volume) {
		volume;
	}

	/** @brief マスターボリュームの取得 (TODO: 正確な値の取得が未実装) */
	float AudioInterface::GetMasterVolume() {
		return 0.0f;
	}

	/** @brief BGMボリュームの取得 (TODO: 未実装) */
	float AudioInterface::GetBGMVolume() {
		return 0.0f;
	}

	/** @brief SEボリュームの取得 (TODO: 未実装) */
	float AudioInterface::GetSEVolume() {
		return 0.0f;
	}

	/** @brief ボイスボリュームの取得 (TODO: 未実装) */
	float AudioInterface::GetVoiceVolume() {
		return 0.0f;
	}

	/** @brief ASボリュームの取得 (TODO: 未実装) */
	float AudioInterface::GetASVolume() {
		return 0.0f;
	}

}
