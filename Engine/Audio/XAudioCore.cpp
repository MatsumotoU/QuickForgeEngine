#include "XAudioCore.h"
#include <cassert>
#include <fstream>

#include "RiffHeader.h"
#include "FormatChunk.h"
#include "ChunkHeader.h"

#include <mmdeviceapi.h>
#include <propsys.h> 
#include <functiondiscoverykeys_devpkey.h> 
#include <string>
#include <iostream>

#pragma comment(lib,"xaudio2.lib")

// デストラクタ
XAudioCore::~XAudioCore() {
	xAudio2_.Reset();
}

// 初期化
void XAudioCore::Initialize() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// XAudio2の初期化
	masterVoice_ = nullptr;
	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	// チャンネルを自動で選択します
	/*hr = xAudio2_.Get()->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(hr));*/
	hr = xAudio2_.Get()->CreateMasteringVoice(&masterVoice_,2);
	assert(SUCCEEDED(hr));

#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"MasterVoice->nChannels:{}", GetOutputChannels())));

	// 音声デバイスを表示
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice* pEndpoint = NULL;
	IPropertyStore* pProps = NULL;
	PROPVARIANT varName;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
	assert(SUCCEEDED(hr));
	hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
	assert(SUCCEEDED(hr));

	UINT count;
	hr = pCollection->GetCount(&count);
	assert(SUCCEEDED(hr));

	if (count == 0)
	{
		DebugLog("No active rendering audio endpoints found.");
	} else
	{
		DebugLog("Active Rendering Audio Endpoints");
		for (UINT i = 0; i < count; i++)
		{
			// 4. コレクションから個々のエンドポイントを取得
			hr = pCollection->Item(i, &pEndpoint);
			assert(SUCCEEDED(hr));

			// 5. デバイスのプロパティストアを開く
			// デバイスの様々なプロパティ（名前、説明など）にアクセスするために必要
			hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
			assert(SUCCEEDED(hr));

			// PROPVARIANT の初期化
			PropVariantInit(&varName);

			// 6. デバイスの表示名 (Friendly Name) を取得
			// PKEY_Device_FriendlyName はデバイスの表示名を識別するプロパティキー
			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
			assert(SUCCEEDED(hr));

			// 7. 取得したプロパティ値を表示
			if (varName.vt == VT_LPWSTR) // 値の型がワイド文字列か確認
			{
				DebugLog("ActiveAudioDeviceName: " + ConvertString(varName.pwszVal));
			} else
			{
				DebugLog("ActiveAudioDeviceName = Unknown format");
			}

			// PROPVARIANT の解放 (CoTaskMemFree を呼び出す)
			PropVariantClear(&varName);

			// リソースの解放
			pProps->Release();
			pEndpoint->Release();
		}
	}
	pEnumerator->Release();
	pCollection->Release();

#endif // _DEBUG

	// X3DAudioの初期化
	audio3D_.Initialize(this);

	multiAudioLoader_.Initialize();
}

// マスターボリュームの設定
void XAudioCore::SetMasterVolume(float volume) {
	masterVoice_->SetVolume(volume);
}

// 出力チャンネル数の取得
uint32_t XAudioCore::GetOutputChannels() {
	DWORD channelMask = 0;
	masterVoice_->GetChannelMask(&channelMask);

	// チャンネル数を数える
	uint32_t channelCount = 0;
	for (uint32_t i = 0; i < 32; ++i) {
		if (channelMask & (1 << i)) channelCount++;
	}
	return channelCount;
}

// マスターボリュームの取得
IXAudio2MasteringVoice* XAudioCore::GetMasterVoice() {
	return masterVoice_;
}

Audio3D* XAudioCore::GetAudio3D() { return &audio3D_; }

// サウンドデータのロード
SoundData Audiomanager::SoundLoad(const char* filename) {
	SoundData mp3Sound{};
	mp3Sound = Multiaudioloader::LoadSoundData(filename);
	return mp3Sound;
}

// サウンドデータのアンロード
void Audiomanager::SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer;
	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

// サウンドデータの再生
void Audiomanager::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData,float volume, float pitch) {
	HRESULT hr;
	IXAudio2SourceVoice* pSourceVoice = CreateSourceVoice(xAudio2, soundData);

	hr = pSourceVoice->SetVolume(volume);
	assert(SUCCEEDED(hr));

	hr = pSourceVoice->SetFrequencyRatio(pitch);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->Start();
	assert(SUCCEEDED(hr));
}

void Audiomanager::SoundPlaySourceVoice(const SoundData& soundData,IXAudio2SourceVoice* pSourceVoice, bool isLoop) {
	HRESULT hr{};

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	if (isLoop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		buf.LoopBegin = 0;
		buf.LoopLength = 0;
	}

	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->Start();
	assert(SUCCEEDED(hr));
}

void Audiomanager::SoundPlayLoopSourceVoice(const SoundData& soundData, IXAudio2SourceVoice* pSourceVoice, float loopBeginSecond, float loopSecond, uint32_t loopCount) {
	HRESULT hr{};

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// ループの設定
	UINT32 sampleRate = soundData.wfex.nSamplesPerSec;
	buf.LoopBegin = static_cast<UINT32>(loopBeginSecond * sampleRate);
	buf.LoopLength = static_cast<UINT32>(loopSecond * sampleRate);
	buf.LoopCount = loopCount;

	// ループ終了位置までしか再生しない
	buf.PlayBegin = 0;
	buf.PlayLength = buf.LoopBegin + buf.LoopLength * buf.LoopCount;
	// 再生する長さバッファが素材本来のバッファを超えないようにする
	assert(soundData.wfex.nBlockAlign != 0);
	UINT32 maxSamples = soundData.bufferSize / soundData.wfex.nBlockAlign;
	buf.PlayLength = std::clamp(buf.PlayLength, static_cast<UINT32>(0), maxSamples);

	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->Start();
	assert(SUCCEEDED(hr));
}

IXAudio2SourceVoice* Audiomanager::CreateSourceVoice(IXAudio2* xAudio2, const SoundData& soundData) {
	IXAudio2SourceVoice* pSourceVoice = nullptr;
#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"Creating SourceVoice with channels:{}", soundData.wfex.nChannels)));
#endif
	HRESULT hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex, XAUDIO2_VOICE_USEFILTER, XAUDIO2_DEFAULT_FREQ_RATIO);
	assert(SUCCEEDED(hr));
	return pSourceVoice;
}

bool Audiomanager::GetIsPlaying(IXAudio2SourceVoice* pSourceVoice) {
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	return state.BuffersQueued > 0;
}
