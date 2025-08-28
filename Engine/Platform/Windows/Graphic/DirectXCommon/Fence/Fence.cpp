#include "Fence.h"
#include <cassert>

void Fence::Initialize(ID3D12Device* device) {
	// Fenceの生成
	currentValue_ = 0;
	HRESULT result = device->CreateFence(currentValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(result) && "Fence creation failed.");
	result;
	// Eventの生成
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ && "Failed to create fence event.");
}

void Fence::Shutdown() {
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
}

void Fence::Signal(ID3D12CommandQueue* queue) {
	// コマンドキューにシグナルを送る
	HRESULT result = queue->Signal(fence_.Get(), ++currentValue_);
	assert(SUCCEEDED(result) && "Failed to signal the command queue.");
	result;
}

void Fence::Wait() {
	// GPUがFenceの値に到達するまで待つ
	if (fence_->GetCompletedValue() < currentValue_) {
		HRESULT result = fence_->SetEventOnCompletion(currentValue_, fenceEvent_);
		assert(SUCCEEDED(result) && "Failed to set event on fence completion.");
		result;
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}
