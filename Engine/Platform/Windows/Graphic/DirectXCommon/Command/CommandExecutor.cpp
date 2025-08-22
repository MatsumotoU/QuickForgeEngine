#include "CommandExecutor.h"

void CommandExecutor::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
	commandList_.Initialize(device, type);
	commandQueue_.Initialize(device, type);
}

void CommandExecutor::ResetCommandList() {
	commandList_.Reset();
}

void CommandExecutor::ExecuteCommandList() {
	commandQueue_.ExecuteCommandList(commandList_.GetCommandList());
}

ID3D12GraphicsCommandList* CommandExecutor::GetCommandList() const {
	return commandList_.GetCommandList();
}

ID3D12CommandQueue* CommandExecutor::GetCommandQueue() const {
	return commandQueue_.GetCommandQueue();
}

const D3D12_COMMAND_LIST_TYPE& CommandExecutor::GetCommandType() const {
	return commandQueue_.GetCommandType();
}
