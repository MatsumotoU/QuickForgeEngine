#include "DirectXCommandList.h"
#include <cassert>

void DirectCommandList::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
	assert(device && "Device is not initialized.");
	assert(!commandAllocator_ && "CommandAllocator already generated");
	assert(!commandList_ && "CommandList already generated");

	HRESULT hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr) && "Failed to create command allocator.");
	hr = device->CreateCommandList(0, type, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr) && "Failed to create command list.");
}

void DirectCommandList::Reset() {
	assert(commandAllocator_ && "Command allocator is not initialized.");
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr) && "Failed to reset command allocator.");
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr) && "Failed to reset command list.");
}

ID3D12GraphicsCommandList* DirectCommandList::GetCommandList() const {
	return commandList_.Get();
}
