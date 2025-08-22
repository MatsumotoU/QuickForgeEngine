#include "DirectXCommandManager.h"
#include <cassert>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void DirectXCommandManager::Initialize(ID3D12Device* device) {
#ifdef _DEBUG
	DebugLog("Initialize called.");
#endif // _DEBUG

	assert(device && "Device is not initialized.");
	commandExecutors_[0].Initialize(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void DirectXCommandManager::ResetCommandList() {
#ifdef _DEBUG
	DebugLog("ResetCommandList called.");
#endif // _DEBUG
	assert(!commandExecutors_.empty() && "Command executors are not initialized.");
	for (auto& executor : commandExecutors_) {
		executor.ResetCommandList();
	}
}

void DirectXCommandManager::ExecuteCommandList() {
#ifdef _DEBUG
	DebugLog("ExecuteCommandList called.");
#endif // _DEBUG
	assert(!commandExecutors_.empty() && "Command executors are not initialized.");
	for (auto& executor : commandExecutors_) {
		executor.ExecuteCommandList();
	}
}

ID3D12GraphicsCommandList* DirectXCommandManager::GetCommandList(const D3D12_COMMAND_LIST_TYPE& type) const {
	for (auto& executor : commandExecutors_) {
		if (executor.GetCommandType() == type) {
			return executor.GetCommandList();
		}
	}

	assert(false && "Command list not found for the specified type.");
	return nullptr;
}

ID3D12CommandQueue* DirectXCommandManager::GetCommandQueue(const D3D12_COMMAND_LIST_TYPE& type) const {
	for (auto& executor : commandExecutors_) {
		if (executor.GetCommandType() == type) {
			return executor.GetCommandQueue();
		}
	}

	assert(false && "Command list not found for the specified type.");
	return nullptr;
}
