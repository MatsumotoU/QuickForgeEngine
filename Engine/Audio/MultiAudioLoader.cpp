#include "MultiAudioLoader.h"
#include <assert.h>
#include <vector>

#include "SoundData.h"
#include "../Base/MyString.h"

MultiAudioLoader::~MultiAudioLoader() {
	MFShutdown();
}

void MultiAudioLoader::Initialize() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));
	hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));
}

SoundData Multiaudioloader::LoadSoundData(const std::string& path) {
	SoundData soundData{};

	// ソースリーダーの生成
	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };
	HRESULT hr = MFCreateSourceReaderFromURL(ConvertString(path).c_str(), nullptr, pMFSourceReader.GetAddressOf());
	assert(SUCCEEDED(hr));

	// メディアタイプの取得
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

	// オーディオデータ形式の作成
	WAVEFORMATEX* waveFormat{ nullptr };
	hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &waveFormat, nullptr);
	soundData.wfex = *waveFormat;
	assert(SUCCEEDED(hr));

	// データの読み込み
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
	soundData.pBuffer = new BYTE[mediaData.size()];
	memcpy(soundData.pBuffer, mediaData.data(), mediaData.size());
	soundData.bufferSize = sizeof(BYTE) * static_cast<UINT32>(mediaData.size());
	return soundData;
}
