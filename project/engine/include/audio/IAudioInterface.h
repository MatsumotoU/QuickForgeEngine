#pragma once
#include <cstdint>

class IAudioInterface {
public:
	virtual ~IAudioInterface() = default;
	virtual void Initialize() = 0;
	virtual void Finalize() = 0;

	// 蜈ｨ菴薙・髻ｳ螢ｰ謫堺ｽ・
	/// 蜈ｨ髻ｳ螢ｰ蛛懈ｭ｢
	virtual void StopAllSound() = 0;
	/// 蜈ｨ髻ｳ螢ｰ荳譎ょ●豁｢
	virtual void PauseAllSound() = 0;
	/// 蜈ｨ髻ｳ螢ｰ蜀埼幕
	virtual void ResumeAllSound() = 0;

	// 蛟句挨縺ｮ髻ｳ螢ｰ謫堺ｽ・
	/// 髻ｳ螢ｰ蜀咲函(volume: 0.0 ~ 1.0縺ｮ遽・峇)
	virtual uint32_t PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) = 0;
	/// 髻ｳ螢ｰ蛛懈ｭ｢
	virtual void StopSound(uint32_t soundHandle) = 0;
	/// 髻ｳ螢ｰ荳譎ょ●豁｢
	virtual void PauseSound(uint32_t soundHandle) = 0;
	/// 髻ｳ螢ｰ蜀埼幕
	virtual void ResumeSound(uint32_t soundHandle) = 0;

	// 蜷・浹驥剰ｨｭ螳・
	/// 荳ｻ髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	virtual void SetMasterVolume(float volume) = 0;
	/// BGM髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	virtual void SetBGMVolume(float volume) = 0;
	/// SE髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	virtual void SetSEVolume(float volume) = 0;
	/// 繝懊う繧ｹ髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	virtual void SetVoiceVolume(float volume) = 0;
	/// AS髻ｳ驥剰ｨｭ螳・0.0 ~ 1.0縺ｮ遽・峇)
	virtual void SetASVolume(float volume) = 0;

	// 蜷・浹驥丞叙蠕・
	/// 荳ｻ髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	virtual float GetMasterVolume() = 0;
	/// BGM髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	virtual float GetBGMVolume() = 0;
	/// SE髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	virtual float GetSEVolume() = 0;
	/// 繝懊う繧ｹ髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	virtual float GetVoiceVolume() = 0;
	/// AS髻ｳ驥丞叙蠕・0.0 ~ 1.0縺ｮ遽・峇)
	virtual float GetASVolume() = 0;
};
