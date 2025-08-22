#pragma once
#include <array>
#include "CommandExecutor.h"

class DirectXCommandManager final {
public:
	DirectXCommandManager() = default;
	~DirectXCommandManager() = default;
	void Initialize(ID3D12Device* device);
	void ResetCommandList();
	void ExecuteCommandList();
	ID3D12GraphicsCommandList* GetCommandList(const D3D12_COMMAND_LIST_TYPE& type) const;
	ID3D12CommandQueue* GetCommandQueue(const D3D12_COMMAND_LIST_TYPE& type) const;
private:
	inline static const uint32_t kCommandExecutorCount_ = 1;
	std::array<CommandExecutor, kCommandExecutorCount_> commandExecutors_;
};