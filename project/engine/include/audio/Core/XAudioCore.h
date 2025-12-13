#pragma once
#define XAUDIO2_DEBUG 1

#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>

class XAudioCore final{
public:
	~XAudioCore();
	
public:
	/// xAudio2繧剃ｽ懈・縺励∪縺・
	void Initialize();
	/// xAudio2繧堤ｵゆｺ・＠縺ｾ縺・
	void Finalize();

public:
	/// 蜈ｨ菴薙・髻ｳ驥上ｒ隱ｿ遽縺励∪縺・1.0縺梧ｨ呎ｺ・
	void SetMasterVolume(float volume);
	uint32_t GetOutputChannels();
	IXAudio2MasteringVoice* GetMasterVoice();
	IXAudio2* GetXAudio2();

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;
};
