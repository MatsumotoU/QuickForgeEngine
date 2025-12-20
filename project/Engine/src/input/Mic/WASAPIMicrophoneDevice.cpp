#include "Engine/include/input/Mic/WASAPIMicrophoneDevice.h"
#include <assert.h>
#include <vector>

#include "Engine/include/assets/AudioSource/Data/AudioData.h"

#ifdef _DEBUG

#endif // _DEBUG

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

	CoTaskMemFree(waveFormat); // メモリリークを修正

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
    // メンバーをゼロ初期化
    ZeroMemory(&audioData.wfxEx, sizeof(WAVEFORMATEXTENSIBLE));

    // フォーマット情報を取得
    WAVEFORMATEX* waveFormat = nullptr;
    HRESULT hr = audioClient_->GetMixFormat(&waveFormat);
    assert(SUCCEEDED(hr));

    // 取得したフォーマットをコピー
    // GetMixFormatはWAVEFORMATEXTENSIBLEを返すことがあるので、そのサイズを考慮してコピーする
    if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        memcpy(&audioData.wfxEx, waveFormat, sizeof(WAVEFORMATEXTENSIBLE));
    }
    else {
        // 古いWAVEFORMATEXの場合は、WAVEFORMATEXTENSIBLEに変換して格納
        memcpy(&audioData.wfxEx.Format, waveFormat, sizeof(WAVEFORMATEX));
        audioData.wfxEx.Format.wFormatTag = waveFormat->wFormatTag;
    }
    CoTaskMemFree(waveFormat);

    std::vector<BYTE> buffer;
    UINT32 packetLength = 0;
    hr = captureClient_->GetNextPacketSize(&packetLength);
    assert(SUCCEEDED(hr));

    // パケットがある限りデータを取得
    while (packetLength != 0) {
        BYTE* pData;
        UINT32 numFramesAvailable;
        DWORD flags;
        hr = captureClient_->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        assert(SUCCEEDED(hr));

        // WAVEFORMATEXTENSIBLE構造体のFormatメンバ(WAVEFORMATEX)を参照する
        const WAVEFORMATEX& format = audioData.wfxEx.Format;

        // データが無音かどうか判定
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            BYTE silentValue = 0;
            if (format.wBitsPerSample == 8) {
                // 8bit PCMは無音＝128
                silentValue = 128;
            }
            // 正しいnBlockAlignを参照
            std::vector<BYTE> silentData(numFramesAvailable * format.nBlockAlign, silentValue);
            buffer.insert(buffer.end(), silentData.begin(), silentData.end());
        }
        else {
            // 正しいnBlockAlignを参照
            size_t dataSize = numFramesAvailable * format.nBlockAlign;
            buffer.insert(buffer.end(), pData, pData + dataSize);
        }

        hr = captureClient_->ReleaseBuffer(numFramesAvailable);
        assert(SUCCEEDED(hr));
        hr = captureClient_->GetNextPacketSize(&packetLength);
        assert(SUCCEEDED(hr));
    }

    // バッファが空の場合、無音データを生成
    if (buffer.empty()) {
        // 正しいnBlockAlignとwBitsPerSampleを参照
        const WAVEFORMATEX& format = audioData.wfxEx.Format;
        size_t silentSize = format.nBlockAlign * 256;
        BYTE silentValue = 0;
        if (format.wBitsPerSample == 8) silentValue = 128;
        buffer.resize(silentSize, silentValue);
    }

    // バッファをAudioDataにセット
    audioData.buffer = std::move(buffer);

    return audioData;
}
