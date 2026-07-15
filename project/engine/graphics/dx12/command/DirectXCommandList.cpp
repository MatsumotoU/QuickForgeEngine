#include "DirectXCommandList.h"
#include <cassert>
using namespace QFE::GRAPHIC;
void DirectCommandList::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
	assert(device && "Device is not initialized.");
	assert(!commandAllocator_ && "CommandAllocator already generated");
	assert(!commandList_ && "CommandList already generated");

	HRESULT hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr) && "Failed to create command allocator.");
	hr = device->CreateCommandList(0, type, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr) && "Failed to create command list.");

	// コマンドリスト4のインターフェースを取得
	if(type == D3D12_COMMAND_LIST_TYPE_DIRECT || type == D3D12_COMMAND_LIST_TYPE_COMPUTE || type == D3D12_COMMAND_LIST_TYPE_COPY) {
		hr = commandList_->QueryInterface(IID_PPV_ARGS(&commandList4_));
		assert(SUCCEEDED(hr) && "Failed to get command list4 interface.");
	} else {
		commandList4_ = nullptr; // 他のタイプではコマンドリスト4は使用できない
	}
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

ID3D12GraphicsCommandList4* QFE::GRAPHIC::DirectCommandList::GetCommandList4() const {
	return commandList4_.Get();
}
