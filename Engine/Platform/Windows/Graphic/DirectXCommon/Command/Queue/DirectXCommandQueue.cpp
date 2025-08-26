#include "DirectXCommandQueue.h"
#include <cassert>

void DirectXCommandQueue::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
	assert(device != nullptr && "Device is null");
	assert(!commandQueue_ && "CommandQueue is already generated");

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = type;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr) && "Failed to create command queue");
	type_ = type;
}

void DirectXCommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList* commandList) {
	assert(commandList != nullptr && "CommandList is null");
	assert(commandQueue_ && "CommandQueue is not initialized");
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr) && "Failed to close command list");
	ID3D12CommandList* lists[] = { commandList };
	commandQueue_->ExecuteCommandLists(_countof(lists), lists);
}

ID3D12CommandQueue* DirectXCommandQueue::GetCommandQueue() const {
	return commandQueue_.Get();
}

const D3D12_COMMAND_LIST_TYPE& DirectXCommandQueue::GetCommandType() const {
	return type_;
}
