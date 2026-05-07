#include "engine/include/graphic/DirectXCommon/Command/DirectXCommandManager.h"
#include <cassert>

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF
using namespace QFE;
void DirectXCommandManager::Initialize(ID3D12Device* device) {
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("Initialize called.");
#endif // QFE_OPTIMIZE_OFF

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

ID3D12CommandQueue* DirectXCommandManager::GetCommandQueue(const D3D12_COMMAND_LIST_TYPE& type) const {
	for (auto& executor : commandExecutors_) {
		if (executor.GetCommandType() == type) {
			return executor.GetCommandQueue();
		}
	}

	assert(false && "Command list not found for the specified type.");
	return nullptr;
}
