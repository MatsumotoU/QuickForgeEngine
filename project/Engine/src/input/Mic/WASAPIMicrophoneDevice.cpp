#include "Engine/include/input/Mic/WASAPIMicrophoneDevice.h"
#include <assert.h>
#include <vector>

#include "Engine/include/assets/AudioSource/Data/AudioData.h"

WASAPIMicrophoneDevice::WASAPIMicrophoneDevice()
{
	isCapturing_ = false;
}

void WASAPIMicrophoneDevice::Initialize()
{
	// COMライブラリの初期化
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// マイクロフォンの初期化
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
	assert(SUCCEEDED(hr));

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &microphoneDevice_);
	assert(SUCCEEDED(hr));

	hr = microphoneDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioClient_);
	assert(SUCCEEDED(hr));

	// IAudioClientの初期化
	WAVEFORMATEX* waveFormat;
	hr = audioClient_->GetMixFormat(&waveFormat);
	assert(SUCCEEDED(hr));

	hr = audioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, waveFormat, NULL);
	assert(SUCCEEDED(hr));

	hr = audioClient_->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient_);
	assert(SUCCEEDED(hr));
}

void WASAPIMicrophoneDevice::Finalize()
{
}

void WASAPIMicrophoneDevice::StartCapture()
{
	HRESULT hr = audioClient_->Start();
	assert(SUCCEEDED(hr));
	isCapturing_ = true;
}

void WASAPIMicrophoneDevice::StopCapture()
{
	HRESULT hr = audioClient_->Stop();
	assert(SUCCEEDED(hr));
	isCapturing_ = false;
}

AudioData WASAPIMicrophoneDevice::GetAudioData()
{
    AudioData audioData;

    // フォーマット情報をセット
    WAVEFORMATEX* waveFormat;
    audioClient_->GetMixFormat(&waveFormat);
    audioData.wfex = *waveFormat;

    std::vector<BYTE> buffer;
    UINT32 packetLength = 0;
    HRESULT hr = captureClient_->GetNextPacketSize(&packetLength);
    assert(SUCCEEDED(hr));

	// パケットがある限りデータを取得
    while (packetLength != 0) {
        BYTE* pData;
        UINT32 numFramesAvailable;
        DWORD flags;
        hr = captureClient_->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        assert(SUCCEEDED(hr));

        // データが無音かどうか判定
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            // 無音の場合はゼロ埋め
            std::vector<BYTE> silentData(numFramesAvailable * audioData.wfex.nBlockAlign, 0);
            buffer.insert(buffer.end(), silentData.begin(), silentData.end());
        }
        else {
            size_t dataSize = numFramesAvailable * audioData.wfex.nBlockAlign;
            buffer.insert(buffer.end(), pData, pData + dataSize);
        }

        hr = captureClient_->ReleaseBuffer(numFramesAvailable);
        assert(SUCCEEDED(hr));
        hr = captureClient_->GetNextPacketSize(&packetLength);
        assert(SUCCEEDED(hr));
    }

    // バッファをAudioDataにセット
    audioData.bufferSize = static_cast<unsigned int>(buffer.size());
    audioData.pBuffer = new BYTE[audioData.bufferSize];
    memcpy(audioData.pBuffer, buffer.data(), audioData.bufferSize);

    return audioData;
}
