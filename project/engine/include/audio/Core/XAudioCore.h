#pragma once
#define XAUDIO2_DEBUG 1

#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>

class XAudioCore final{
public:
	~XAudioCore();
	
public:
	/// xAudio2を作成します
	void Initialize();
	/// xAudio2を終了します
	void Finalize();

public:
	/// 全体の音量を調節します(1.0が標準)
	void SetMasterVolume(float volume);
	uint32_t GetOutputChannels();
	IXAudio2MasteringVoice* GetMasterVoice();
	IXAudio2* GetXAudio2();

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;
};