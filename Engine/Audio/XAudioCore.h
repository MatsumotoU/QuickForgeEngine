#pragma once
#define XAUDIO2_DEBUG 1

#include <wrl.h>
#include <xaudio2.h>

#include "Audio3D.h"
#include "MultiAudioLoader.h"
#include "SoundData.h"

#ifdef _DEBUG
#include "Utility/MyString.h"
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

class XAudioCore final{
public:
	~XAudioCore();
	
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
	uint32_t GetOutputChannels();
	IXAudio2MasteringVoice* GetMasterVoice();
	Audio3D* GetAudio3D();

public:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

private:
	IXAudio2MasteringVoice* masterVoice_;
	MultiAudioLoader multiAudioLoader_;
	Audio3D audio3D_;
};

namespace Audiomanager {
	/// <summary>
	/// サウンドデータを読み込みます
	/// </summary>
	/// <param name="filename">filePath</param>
	/// <returns></returns>
	SoundData SoundLoad(const char* filename);
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
	/// <summary>
	/// ソースボイスの設定を適用して音源を鳴らします
	/// </summary>
	/// <param name="xAudio2">XAudio2</param>
	/// <param name="sourceVoice">SourceVoice</param>
	void SoundPlaySourceVoice(const SoundData& soundData, IXAudio2SourceVoice* pSourceVoice,bool isLoop);
	/// <summary>
	/// ソースボイスのループ回数を指定して音源を鳴らします
	/// </summary>
	/// <param name="soundData"></param>
	/// <param name="pSourceVoice"></param>
	/// <param name="loopBeginSecond">ループ開始地点を何秒ずらすか</param>
	/// <param name="loopSecond">ループ開始地点から何秒鳴らすか</param>
	/// <param name="loopCount"></param>
	void SoundPlayLoopSourceVoice(
		const SoundData& soundData, IXAudio2SourceVoice* pSourceVoice, 
		float loopBeginSecond,float loopSecond,uint32_t loopCount);
	/// <summary>
	/// ソースボイスを作成します
	/// </summary>
	/// <param name="xAudio2">xAudio2</param>
	/// <param name="soundData">soundData</param>
	/// <returns>ソースボイス</returns>
	IXAudio2SourceVoice* CreateSourceVoice(IXAudio2* xAudio2, const SoundData& soundData);
	/// <summary>
	/// ソースボイスが再生中かどうかを取得します
	/// </summary>
	/// <param name="pSourceVoice">該当ソースボイス</param>
	/// <returns></returns>
	bool GetIsPlaying(IXAudio2SourceVoice* pSourceVoice);
}