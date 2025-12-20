#pragma once
#include <wrl.h>
#include <mmdeviceapi.h>
#include <AudioClient.h>

#include <vector>

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
	bool IsCapturing() const { return isCapturing_; }

private:
	Microsoft::WRL::ComPtr<IMMDevice> microphoneDevice_;
	Microsoft::WRL::ComPtr<IAudioClient> audioClient_;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient_;

	bool isCapturing_;

	std::vector<UINT32> test;
};