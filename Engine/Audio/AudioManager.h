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
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
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
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);
}