#include "AudioManager.h"
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
AudioManager::~AudioManager() {
	xAudio2_.Reset();
}

// 初期化
void AudioManager::Initialize() {
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
void AudioManager::SetMasterVolume(float volume) {
	masterVoice_->SetVolume(volume);
}

// 出力チャンネル数の取得
uint32_t AudioManager::GetOutputChannels() {
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
IXAudio2MasteringVoice* AudioManager::GetMasterVoice() {
	return masterVoice_;
}

Audio3D* AudioManager::GetAudio3D() { return &audio3D_; }

// サウンドデータのロード
SoundData Audiomanager::SoundLoadWave(const char* filename) {
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());

	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	FormatChunk format = {};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0 && "Error");
	}
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"WAV File Format - Channels: {}, SampleRate: {}, BitsPerSample: {}", 
		format.fmt.nChannels, format.fmt.nSamplesPerSec, format.fmt.wBitsPerSample)));
#endif

	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "LIST", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	file.close();

	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

// mp3データのロード
SoundData Audiomanager::SoundLoadMp3(const char* filename) {
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

void Audiomanager::SoundPlaySourceVoice(const SoundData& soundData,IXAudio2SourceVoice* pSourceVoice) {
	HRESULT hr{};

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

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
