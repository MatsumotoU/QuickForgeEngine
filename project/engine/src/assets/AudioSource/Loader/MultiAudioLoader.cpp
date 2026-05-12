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
	// MFSourceReaderの作成
	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };
	HRESULT hr = MFCreateSourceReaderFromURL(ConvertString(path).c_str(), nullptr, pMFSourceReader.GetAddressOf());
	if(!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("Failed to create source reader from URL: HRESULT = 0x{:08X}", hr),SystemError::Abort);
	}

	// メディアタイプの設定
	IMFMediaType* pMFMediaType{ nullptr };
	hr = MFCreateMediaType(&pMFMediaType);
	if(!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("Failed to create media type: HRESULT = 0x{:08X}", hr),SystemError::Abort);
	}
	hr = pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	if(!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("Failed to set major type: HRESULT = 0x{:08X}", hr),SystemError::Abort);
	}
	hr = pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	if(!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("Failed to set subtype: HRESULT = 0x{:08X}", hr),SystemError::Abort);
	}
	hr = pMFSourceReader.Get()->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType);
	if(!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("Failed to set current media type: HRESULT = 0x{:08X}", hr),SystemError::Abort);
	}

	pMFMediaType->Release();
	pMFMediaType = nullptr;
	hr = pMFSourceReader.Get()->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &pMFMediaType);
	assert(SUCCEEDED(hr));

	// メディアデータの読み込み
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

	// waveformatの取得
	WAVEFORMATEX* waveFormat{ nullptr };
	UINT32 waveFormatSize = 0;
	hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &waveFormat, &waveFormatSize);
	assert(SUCCEEDED(hr));

	AudioData soundData;
	ZeroMemory(&soundData.wfxEx, sizeof(WAVEFORMATEXTENSIBLE));
	if (waveFormatSize == sizeof(WAVEFORMATEXTENSIBLE)) {
		memcpy(&soundData.wfxEx, waveFormat, sizeof(WAVEFORMATEXTENSIBLE));
	}
	else {
		assert(waveFormatSize == sizeof(WAVEFORMATEX));
		memcpy(&soundData.wfxEx.Format, waveFormat, sizeof(WAVEFORMATEX));
	}
	CoTaskMemFree(waveFormat);

	// バッファのコピー
	SafeVector<BYTE> buffer(static_cast<size_t>(mediaData.size()));
	for (size_t i = 0; i < mediaData.size(); i++) {
		buffer.push_back(mediaData.at(i));
	}
	soundData.buffer = std::move(buffer);
	
	return soundData;
}
