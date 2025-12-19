#pragma once
#include "IAudioInterface.h"
#include "Core/XAudioCore.h"
#include "Player/AudioChipManager.h"
#include "engine/include/utility/DesignPatterns/Singleton.h"

class AudioInterface final : public IAudioInterface , public Singleton<AudioInterface>{
	friend class Singleton<AudioInterface>;
public:
	AudioInterface() = default;
	~AudioInterface() override = default;
	void Initialize() override;
	void Finalize() override;

	// 蜈ｨ菴薙・髻ｳ螢ｰ謫堺ｽ・
	/// 蜈ｨ髻ｳ螢ｰ蛛懈ｭ｢
	void StopAllSound() override;
	/// 蜈ｨ髻ｳ螢ｰ荳譎ょ●豁｢
	void PauseAllSound() override;
	/// 蜈ｨ髻ｳ螢ｰ蜀埼幕
	void ResumeAllSound() override;

	// 蛟句挨縺ｮ髻ｳ螢ｰ謫堺ｽ・
	/// 髻ｳ螢ｰ蜀咲函(volume: 0.0 ~ 1.0縺ｮ遽・峇)
	uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) override;
	/// 髻ｳ螢ｰ蛛懈ｭ｢
	void StopSound(uint32_t soundHandle) override;
	/// 髻ｳ螢ｰ荳譎ょ●豁｢
	void PauseSound(uint32_t soundHandle) override;
	/// 髻ｳ螢ｰ蜀埼幕
	void ResumeSound(uint32_t soundHandle) override;

	// 蜷・浹驥剰ｨｭ螳・
	/// 荳ｻ髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	void SetMasterVolume(float volume) override;
	/// BGM髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	void SetBGMVolume(float volume) override;
	/// SE髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	void SetSEVolume(float volume) override;
	/// 繝懊う繧ｹ髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	void SetVoiceVolume(float volume) override;
	/// 迺ｰ蠅・浹驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	void SetASVolume(float volume) override;

	// 蜷・浹驥丞叙蠕・
	/// 荳ｻ髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	float GetMasterVolume() override;
	/// BGM髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	float GetBGMVolume() override;
	/// SE髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	float GetSEVolume() override;
	/// 繝懊う繧ｹ髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	float GetVoiceVolume() override;
	/// 迺ｰ蠅・浹驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	float GetASVolume() override;

private:
	XAudioCore xAudioCore_;
	AudioChipManager audioChipManager_;
};
