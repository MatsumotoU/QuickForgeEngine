#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class Fence final {
public:
	Fence() = default;
	~Fence() = default;
	void Initialize(ID3D12Device* device);
	void Shutdown();
	void Signal(ID3D12CommandQueue* queue);
	void Wait();
	uint64_t GetCurrentValue() const { return currentValue_; }
private:
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t currentValue_;
	HANDLE fenceEvent_;
};