/**
 * @file XAudioCore.cpp
 * @brief XAudio2の基盤部分(デバイス生成等)を管理するクラスの実装
 */

#include "engine/include/audio/Core/XAudioCore.h"
#include <cassert>
#include <fstream>

#include "engine/include/audio/Core/RiffHeader.h"
#include "engine/include/audio/Core/FormatChunk.h"
#include "engine/include/audio/Core/ChunkHeader.h"

#include <mmdeviceapi.h>
#include <propsys.h> 
#include <functiondiscoverykeys_devpkey.h> 
#include <string>
#include <iostream>
#include <algorithm>
#include <format>

#pragma comment(lib,"xaudio2.lib")

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/MyString.h"
#endif // _DEBUG

/** @brief デストラクタ */
XAudioCore::~XAudioCore() {
	xAudio2_.Reset();
}

/** @brief 初期化 */
void XAudioCore::Initialize() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// XAudio2の初期化
	masterVoice_ = nullptr;
	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	// チャンネル数は2で固定
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
			// コレクションから個別のエンドポイントを取得
			hr = pCollection->Item(i, &pEndpoint);
			assert(SUCCEEDED(hr));

			// デバイスのプロパティストアを開く
			// デバイスの様々なプロパティ(名前、説明など)にアクセスするために必要
			hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
			assert(SUCCEEDED(hr));

			// PROPVARIANT の初期化
			PropVariantInit(&varName);

			// デバイスの表示名(Friendly Name) を取得
			// PKEY_Device_FriendlyName はデバイスの表示名を識別するプロパティキー
			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
			assert(SUCCEEDED(hr));

			// 取得したプロパティ値を表示
			if (varName.vt == VT_LPWSTR) // 値の型がワイド文字列か確認
			{
				DebugLog("ActiveAudioDeviceName: " + ConvertString(varName.pwszVal));
			} else
			{
				DebugLog("ActiveAudioDeviceName = Unknown format");
			}

			// PROPVARIANT の解放
			PropVariantClear(&varName);

			// リソースの解放
			pProps->Release();
			pEndpoint->Release();
		}
	}
	pEnumerator->Release();
	pCollection->Release();

#endif // _DEBUG
}

void XAudioCore::Finalize() {
}

/** @brief マスターボリュームの設定 */
void XAudioCore::SetMasterVolume(float volume) {
	masterVoice_->SetVolume(volume);
}

/** @brief 出力チャンネル数の取得 */
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

// 繝槭せ繧ｿ繝ｼ繝懊Μ繝･繝ｼ繝縺ｮ蜿門ｾ・
IXAudio2MasteringVoice* XAudioCore::GetMasterVoice() {
	return masterVoice_;
}

IXAudio2* XAudioCore::GetXAudio2() {
	return xAudio2_.Get();
}
