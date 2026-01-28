#include "engine/include/assets/AudioSource/Loader/MultiAudioLoader.h"
#include <assert.h>
#include <vector>

#include "engine/include/assets/AudioSource/Data/AudioData.h"
#include "engine/include/utility/String/MyString.h"

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
using namespace QFE;

void Multiaudioloader::Initialize() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));
	hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));
}

void Multiaudioloader::Finalize() {
	MFShutdown();
}

AudioData Multiaudioloader::LoadAudioData(const std::string& path) {
	AudioData soundData{};

	// 繧ｽ繝ｼ繧ｹ繝ｪ繝ｼ繝繝ｼ縺ｮ逕滓・
	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };
	HRESULT hr = MFCreateSourceReaderFromURL(ConvertString(path).c_str(), nullptr, pMFSourceReader.GetAddressOf());
	assert(SUCCEEDED(hr));

	// 繝｡繝・ぅ繧｢繧ｿ繧､繝励・蜿門ｾ・
	IMFMediaType* pMFMediaType{ nullptr };
	hr = MFCreateMediaType(&pMFMediaType);
	assert(SUCCEEDED(hr));
	hr = pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	assert(SUCCEEDED(hr));
	hr = pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	assert(SUCCEEDED(hr));
	hr = pMFSourceReader.Get()->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType);
	assert(SUCCEEDED(hr));

	pMFMediaType->Release();
	pMFMediaType = nullptr;
	hr = pMFSourceReader.Get()->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &pMFMediaType);
	assert(SUCCEEDED(hr));

	// 繧ｪ繝ｼ繝・ぅ繧ｪ繝・・繧ｿ蠖｢蠑上・菴懈・
	WAVEFORMATEX* waveFormat{ nullptr };
	UINT32 waveFormatSize = 0;
	hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &waveFormat, &waveFormatSize);
	assert(SUCCEEDED(hr));

	ZeroMemory(&soundData.wfxEx, sizeof(WAVEFORMATEXTENSIBLE));
    if (waveFormatSize == sizeof(WAVEFORMATEXTENSIBLE)) {
        memcpy(&soundData.wfxEx, waveFormat, sizeof(WAVEFORMATEXTENSIBLE));
    }
    else {
        assert(waveFormatSize == sizeof(WAVEFORMATEX));
        memcpy(&soundData.wfxEx.Format, waveFormat, sizeof(WAVEFORMATEX));
    }

	CoTaskMemFree(waveFormat);

	// 繝・・繧ｿ縺ｮ隱ｭ縺ｿ霎ｼ縺ｿ
	std::vector<BYTE> mediaData;
	while (true)
	{
		IMFSample* pMFSample{ nullptr };
		DWORD dwStreamFlags{ 0 };
		hr = pMFSourceReader.Get()->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);
		assert(SUCCEEDED(hr));

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			break;
		}

		IMFMediaBuffer* pMFMediaBuffer{ nullptr };
		hr = pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);
		assert(SUCCEEDED(hr));

		BYTE* pBuffer{ nullptr };
		DWORD cbCurrentLength{ 0 };
		hr = pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);
		assert(SUCCEEDED(hr));

		mediaData.resize(mediaData.size() + cbCurrentLength);
		memcpy(mediaData.data() + mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);

		pMFMediaBuffer->Unlock();

		pMFMediaBuffer->Release();
		pMFSample->Release();
	}
	soundData.buffer = std::move(mediaData);
	return soundData;
}
