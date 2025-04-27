#pragma once
#include <wrl.h>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include "SoundData.h"

class AudioManager {
public:
	~AudioManager();
	
public:
	/// <summary>
	/// xAudio2を作成します
	/// </summary>
	void Initialize();

public:
	/// <summary>
	/// 全体の音量を調節します(1.0が標準)
	/// </summary>
	/// <param name="volume"></param>
	void SetMasterVolume(float volume);

public:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

private:
	IXAudio2MasteringVoice* masterVoice_;
};

namespace Audiomanager {
	/// <summary>
	/// サウンドデータを読み込みます
	/// </summary>
	/// <param name="filename">filePath</param>
	/// <returns></returns>
	SoundData SoundLoadWave(const char* filename);
	/// <summary>
	/// サウンドデータを削除します
	/// </summary>
	/// <param name="soundData">soundData</param>
	void SoundUnload(SoundData* soundData);
	/// <summary>
	/// 音源を鳴らします
	/// </summary>
	/// <param name="xAudio2">xAudio2</param>
	/// <param name="soundData">soundData</param>
	/// <param name="volume">音量(基準:1.0)</param>
	/// <param name="pitch">ピッチというよりかは再生速度(基準:1.0)</param>
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData, float volume,float pitch);
}