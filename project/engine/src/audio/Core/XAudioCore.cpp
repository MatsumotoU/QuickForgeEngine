/**
 * @file XAudioCore.cpp
 * @brief XAudio2縺ｮ蝓ｺ逶､驛ｨ蛻・繝・ヰ繧､繧ｹ逕滓・遲・繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ縺ｮ螳溯｣・
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
#include "engine/include/core/EngineDefines.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/String/MyString.h"
#endif // QFE_OPTIMIZE_OFF
using namespace QFE;
/** @brief 繝・せ繝医Λ繧ｯ繧ｿ */
XAudioCore::~XAudioCore() {
	xAudio2_.Reset();
}

/** @brief 蛻晄悄蛹・*/
void XAudioCore::Initialize() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// XAudio2縺ｮ蛻晄悄蛹・
	masterVoice_ = nullptr;
	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	// 繝√Ε繝ｳ繝阪Ν謨ｰ縺ｯ2縺ｧ蝗ｺ螳・
	hr = xAudio2_.Get()->CreateMasteringVoice(&masterVoice_,2);
	assert(SUCCEEDED(hr));

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG(ConvertString(std::format(L"MasterVoice->nChannels:{}", GetOutputChannels())));

	// 髻ｳ螢ｰ繝・ヰ繧､繧ｹ繧定｡ｨ遉ｺ
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
		QFE_LOG("No active rendering audio endpoints found.");
	} else
	{
		QFE_LOG("Active Rendering Audio Endpoints");
		for (UINT i = 0; i < count; i++)
		{
			// 繧ｳ繝ｬ繧ｯ繧ｷ繝ｧ繝ｳ縺九ｉ蛟句挨縺ｮ繧ｨ繝ｳ繝峨・繧､繝ｳ繝医ｒ蜿門ｾ・
			hr = pCollection->Item(i, &pEndpoint);
			assert(SUCCEEDED(hr));

			// 繝・ヰ繧､繧ｹ縺ｮ繝励Ο繝代ユ繧｣繧ｹ繝医い繧帝幕縺・
			// 繝・ヰ繧､繧ｹ縺ｮ讒倥・↑繝励Ο繝代ユ繧｣(蜷榊燕縲∬ｪｬ譏弱↑縺ｩ)縺ｫ繧｢繧ｯ繧ｻ繧ｹ縺吶ｋ縺溘ａ縺ｫ蠢・ｦ・
			hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
			assert(SUCCEEDED(hr));

			// PROPVARIANT 縺ｮ蛻晄悄蛹・
			PropVariantInit(&varName);

			// 繝・ヰ繧､繧ｹ縺ｮ陦ｨ遉ｺ蜷・Friendly Name) 繧貞叙蠕・
			// PKEY_Device_FriendlyName 縺ｯ繝・ヰ繧､繧ｹ縺ｮ陦ｨ遉ｺ蜷阪ｒ隴伜挨縺吶ｋ繝励Ο繝代ユ繧｣繧ｭ繝ｼ
			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
			assert(SUCCEEDED(hr));

			// 蜿門ｾ励＠縺溘・繝ｭ繝代ユ繧｣蛟､繧定｡ｨ遉ｺ
			if (varName.vt == VT_LPWSTR) // 蛟､縺ｮ蝙九′繝ｯ繧､繝画枚蟄怜・縺狗｢ｺ隱・
			{
				QFE_LOG("ActiveAudioDeviceName: " + ConvertString(varName.pwszVal));
			} else
			{
				QFE_LOG("ActiveAudioDeviceName = Unknown format");
			}

			// PROPVARIANT 縺ｮ隗｣謾ｾ
			PropVariantClear(&varName);

			// 繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ隗｣謾ｾ
			pProps->Release();
			pEndpoint->Release();
		}
	}
	pEnumerator->Release();
	pCollection->Release();

#endif // QFE_OPTIMIZE_OFF
}

void XAudioCore::Finalize() {
}

/** @brief 繝槭せ繧ｿ繝ｼ繝懊Μ繝･繝ｼ繝縺ｮ險ｭ螳・*/
void XAudioCore::SetMasterVolume(float volume) {
	masterVoice_->SetVolume(volume);
}

/** @brief 蜃ｺ蜉帙メ繝｣繝ｳ繝阪Ν謨ｰ縺ｮ蜿門ｾ・*/
uint32_t XAudioCore::GetOutputChannels() {
	DWORD channelMask = 0;
	masterVoice_->GetChannelMask(&channelMask);

	// 繝√Ε繝ｳ繝阪Ν謨ｰ繧呈焚縺医ｋ
	uint32_t channelCount = 0;
	for (uint32_t i = 0; i < 32; ++i) {
		if (channelMask & (1 << i)) channelCount++;
	}
	return channelCount;
}

// 郢晄ｧｭ縺帷ｹｧ・ｿ郢晢ｽｼ郢晄㈱ﾎ懃ｹ晢ｽ･郢晢ｽｼ郢晢｣ｰ邵ｺ・ｮ陷ｿ髢・ｾ繝ｻ
IXAudio2MasteringVoice* XAudioCore::GetMasterVoice() {
	return masterVoice_;
}

IXAudio2* XAudioCore::GetXAudio2() {
	return xAudio2_.Get();
}


