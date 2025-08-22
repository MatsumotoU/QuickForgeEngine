#pragma once
#include "List/DirectXCommandList.h"
#include "Queue/DirectXCommandQueue.h"

class CommandExecutor final {
public:
	CommandExecutor() = default;
	~CommandExecutor() = default;
	void Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
	void ResetCommandList();
	void ExecuteCommandList();
	ID3D12GraphicsCommandList* GetCommandList() const;
	ID3D12CommandQueue* GetCommandQueue() const;
	const D3D12_COMMAND_LIST_TYPE& GetCommandType() const;

private:
	DirectCommandList commandList_;
	DirectXCommandQueue commandQueue_;
};