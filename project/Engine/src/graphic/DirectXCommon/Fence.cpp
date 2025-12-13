#include "engine/include/graphic/DirectXCommon/Fence.h"
#include <cassert>

void Fence::Initialize(ID3D12Device* device) {
	// Fence縺ｮ逕滓・
	currentValue_ = 0;
	HRESULT result = device->CreateFence(currentValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(result) && "Fence creation failed.");
	result;
	// Event縺ｮ逕滓・
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
	// 繧ｳ繝槭Φ繝峨く繝･繝ｼ縺ｫ繧ｷ繧ｰ繝翫Ν繧帝√ｋ
	HRESULT result = queue->Signal(fence_.Get(), ++currentValue_);
	assert(SUCCEEDED(result) && "Failed to signal the command queue.");
	result;
}

void Fence::Wait() {
	// GPU縺熊ence縺ｮ蛟､縺ｫ蛻ｰ驕斐☆繧九∪縺ｧ蠕・▽
	if (fence_->GetCompletedValue() < currentValue_) {
		HRESULT result = fence_->SetEventOnCompletion(currentValue_, fenceEvent_);
		assert(SUCCEEDED(result) && "Failed to set event on fence completion.");
		result;
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}
