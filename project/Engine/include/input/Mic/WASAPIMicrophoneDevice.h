#pragma once
#include <wrl.h>
#include <mmdeviceapi.h>
#include <AudioClient.h>

struct AudioData;

class WASAPIMicrophoneDevice final{
public:
	WASAPIMicrophoneDevice();
	~WASAPIMicrophoneDevice() = default;

	void Initialize();
	void Finalize();

	void StartCapture();
	void StopCapture();

	AudioData GetAudioData();

private:
	Microsoft::WRL::ComPtr<IMMDevice> microphoneDevice_;
	Microsoft::WRL::ComPtr<IAudioClient> audioClient_;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient_;

	bool isCapturing_;
};