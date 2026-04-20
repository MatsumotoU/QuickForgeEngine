#include "Engine/include/input/Mic/WASAPIMicrophoneDevice.h"
#include <assert.h>
#include <vector>

#include "Engine/include/assets/AudioSource/Data/AudioData.h"
using namespace QFE;
#ifdef QFE_OPTIMIZE_OFF

#endif // QFE_OPTIMIZE_OFF

WASAPIMicrophoneDevice::WASAPIMicrophoneDevice()
{
	isCapturing_ = false;
}

void WASAPIMicrophoneDevice::Initialize()
{
	// COM繝ｩ繧､繝悶Λ繝ｪ縺ｮ蛻晄悄蛹・
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// 繝槭う繧ｯ繝ｭ繝輔か繝ｳ縺ｮ蛻晄悄蛹・
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
	assert(SUCCEEDED(hr));

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &microphoneDevice_);
	assert(SUCCEEDED(hr));

	hr = microphoneDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioClient_);
	assert(SUCCEEDED(hr));

	// IAudioClient縺ｮ蛻晄悄蛹・
	WAVEFORMATEX* waveFormat;
	hr = audioClient_->GetMixFormat(&waveFormat);
	assert(SUCCEEDED(hr));

	hr = audioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, waveFormat, NULL);
	assert(SUCCEEDED(hr));

	CoTaskMemFree(waveFormat); // 繝｡繝｢繝ｪ繝ｪ繝ｼ繧ｯ繧剃ｿｮ豁｣

	hr = audioClient_->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient_);
	assert(SUCCEEDED(hr));
}

void WASAPIMicrophoneDevice::Finalize()
{
}

void WASAPIMicrophoneDevice::StartCapture()
{
	HRESULT hr = audioClient_->Start();
    hr;
	assert(SUCCEEDED(hr));
	isCapturing_ = true;
}

void WASAPIMicrophoneDevice::StopCapture()
{
	HRESULT hr = audioClient_->Stop();
    hr;
	assert(SUCCEEDED(hr));
	isCapturing_ = false;
}

AudioData WASAPIMicrophoneDevice::GetAudioData()
{
    AudioData audioData;
    // バッファの初期化
    ZeroMemory(&audioData.wfxEx, sizeof(WAVEFORMATEXTENSIBLE));

	// waveFormatの取得
    WAVEFORMATEX* waveFormat = nullptr;
    HRESULT hr = audioClient_->GetMixFormat(&waveFormat);
    assert(SUCCEEDED(hr));

    // 取得したwaveFormatをAudioDataにコピー
    if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        memcpy(&audioData.wfxEx, waveFormat, sizeof(WAVEFORMATEXTENSIBLE));
    }
    else {
        // WAVEFORMATEXの場合
        memcpy(&audioData.wfxEx.Format, waveFormat, sizeof(WAVEFORMATEX));
        audioData.wfxEx.Format.wFormatTag = waveFormat->wFormatTag;
    }
    CoTaskMemFree(waveFormat);

    std::vector<BYTE> buffer;
    UINT32 packetLength = 0;
    hr = captureClient_->GetNextPacketSize(&packetLength);
    assert(SUCCEEDED(hr));

    // パケットが存在する限りループ
    while (packetLength != 0) {
        BYTE* pData;
        UINT32 numFramesAvailable;
        DWORD flags;
        hr = captureClient_->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        assert(SUCCEEDED(hr));

		// フォーマット情報の取得
        const WAVEFORMATEX& format = audioData.wfxEx.Format;

		// SILENTフラグが立っている場合は、サイレントデータを挿入
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            BYTE silentValue = 0;
            if (format.wBitsPerSample == 8) {
                // 8bit PCMの場合は128がサイレント値
                silentValue = 128;
            }
			// サイレントデータの生成
            std::vector<BYTE> silentData(numFramesAvailable * format.nBlockAlign, silentValue);
            buffer.insert(buffer.end(), silentData.begin(), silentData.end());
        }
        else {
            // 通常のオーディオデータのコピー
            size_t dataSize = numFramesAvailable * format.nBlockAlign;
            buffer.insert(buffer.end(), pData, pData + dataSize);
        }

        hr = captureClient_->ReleaseBuffer(numFramesAvailable);
        assert(SUCCEEDED(hr));
        hr = captureClient_->GetNextPacketSize(&packetLength);
        assert(SUCCEEDED(hr));
    }

	// バッファが空の場合は、サイレントデータを挿入
    if (buffer.empty()) {
		// SILENTフラグが立っている場合は、サイレントデータを挿入
        const WAVEFORMATEX& format = audioData.wfxEx.Format;
        size_t silentSize = format.nBlockAlign * 256;
        BYTE silentValue = 0;
        if (format.wBitsPerSample == 8) silentValue = 128;
        buffer.resize(silentSize, silentValue);
    }

    // バッファのコピー
	SafeVector<BYTE> safeBuffer(buffer.size());
    for (size_t i = 0; i < buffer.size(); ++i) {
        safeBuffer.push_back(buffer[i]);
	}
    audioData.buffer = std::move(safeBuffer);

    return audioData;
}


