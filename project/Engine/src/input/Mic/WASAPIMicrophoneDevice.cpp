#include "Engine/include/input/Mic/WASAPIMicrophoneDevice.h"
#include <assert.h>
#include <vector>

#include "Engine/include/assets/AudioSource/Data/AudioData.h"

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
    // 繝｡繝ｳ繝舌・繧偵ぞ繝ｭ蛻晄悄蛹・
    ZeroMemory(&audioData.wfxEx, sizeof(WAVEFORMATEXTENSIBLE));

    // 繝輔か繝ｼ繝槭ャ繝域ュ蝣ｱ繧貞叙蠕・
    WAVEFORMATEX* waveFormat = nullptr;
    HRESULT hr = audioClient_->GetMixFormat(&waveFormat);
    assert(SUCCEEDED(hr));

    // 蜿門ｾ励＠縺溘ヵ繧ｩ繝ｼ繝槭ャ繝医ｒ繧ｳ繝斐・
    // GetMixFormat縺ｯWAVEFORMATEXTENSIBLE繧定ｿ斐☆縺薙→縺後≠繧九・縺ｧ縲√◎縺ｮ繧ｵ繧､繧ｺ繧定・・縺励※繧ｳ繝斐・縺吶ｋ
    if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        memcpy(&audioData.wfxEx, waveFormat, sizeof(WAVEFORMATEXTENSIBLE));
    }
    else {
        // 蜿､縺ЦAVEFORMATEX縺ｮ蝣ｴ蜷医・縲仝AVEFORMATEXTENSIBLE縺ｫ螟画鋤縺励※譬ｼ邏・
        memcpy(&audioData.wfxEx.Format, waveFormat, sizeof(WAVEFORMATEX));
        audioData.wfxEx.Format.wFormatTag = waveFormat->wFormatTag;
    }
    CoTaskMemFree(waveFormat);

    std::vector<BYTE> buffer;
    UINT32 packetLength = 0;
    hr = captureClient_->GetNextPacketSize(&packetLength);
    assert(SUCCEEDED(hr));

    // 繝代こ繝・ヨ縺後≠繧矩剞繧翫ョ繝ｼ繧ｿ繧貞叙蠕・
    while (packetLength != 0) {
        BYTE* pData;
        UINT32 numFramesAvailable;
        DWORD flags;
        hr = captureClient_->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        assert(SUCCEEDED(hr));

        // WAVEFORMATEXTENSIBLE讒矩菴薙・Format繝｡繝ｳ繝・WAVEFORMATEX)繧貞盾辣ｧ縺吶ｋ
        const WAVEFORMATEX& format = audioData.wfxEx.Format;

        // 繝・・繧ｿ縺檎┌髻ｳ縺九←縺・°蛻､螳・
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            BYTE silentValue = 0;
            if (format.wBitsPerSample == 8) {
                // 8bit PCM縺ｯ辟｡髻ｳ・・28
                silentValue = 128;
            }
            // 豁｣縺励＞nBlockAlign繧貞盾辣ｧ
            std::vector<BYTE> silentData(numFramesAvailable * format.nBlockAlign, silentValue);
            buffer.insert(buffer.end(), silentData.begin(), silentData.end());
        }
        else {
            // 豁｣縺励＞nBlockAlign繧貞盾辣ｧ
            size_t dataSize = numFramesAvailable * format.nBlockAlign;
            buffer.insert(buffer.end(), pData, pData + dataSize);
        }

        hr = captureClient_->ReleaseBuffer(numFramesAvailable);
        assert(SUCCEEDED(hr));
        hr = captureClient_->GetNextPacketSize(&packetLength);
        assert(SUCCEEDED(hr));
    }

    // 繝舌ャ繝輔ぃ縺檎ｩｺ縺ｮ蝣ｴ蜷医∫┌髻ｳ繝・・繧ｿ繧堤函謌・
    if (buffer.empty()) {
        // 豁｣縺励＞nBlockAlign縺ｨwBitsPerSample繧貞盾辣ｧ
        const WAVEFORMATEX& format = audioData.wfxEx.Format;
        size_t silentSize = format.nBlockAlign * 256;
        BYTE silentValue = 0;
        if (format.wBitsPerSample == 8) silentValue = 128;
        buffer.resize(silentSize, silentValue);
    }

    // 繝舌ャ繝輔ぃ繧但udioData縺ｫ繧ｻ繝・ヨ
    audioData.buffer = std::move(buffer);

    return audioData;
}


