#include "DirectXCommandManager.h"
#include <cassert>

#include "EngineDefines.h"

using namespace QFE::GRAPHIC;
void DirectXCommandManager::Initialize(ID3D12Device* device) {
	QFE_LOG("Initialize called.");

	assert(device && "Device is not initialized.");
	commandExecutors_[0].Initialize(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void DirectXCommandManager::ResetCommandList() {
	assert(!commandExecutors_.empty() && "Command executors are not initialized.");
	for (auto& executor : commandExecutors_) {
		executor.ResetCommandList();
	}
}

void DirectXCommandManager::ExecuteCommandList() {
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

ID3D12GraphicsCommandList4* QFE::GRAPHIC::DirectXCommandManager::GetCommandList4(const D3D12_COMMAND_LIST_TYPE& type) const {
	for (auto& executor : commandExecutors_) {
		if (executor.GetCommandType() == type) {
			return executor.GetCommandList4();
		}
	}
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
