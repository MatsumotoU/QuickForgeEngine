#pragma once
#include <d3d12.h>
#include <wrl.h>

class DirectXCommandQueue final {
public:
	DirectXCommandQueue() = default;
	~DirectXCommandQueue() = default;
	void Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
	void ExecuteCommandList(ID3D12GraphicsCommandList* commandList);
	ID3D12CommandQueue* GetCommandQueue() const;
	const D3D12_COMMAND_LIST_TYPE& GetCommandType() const;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	D3D12_COMMAND_LIST_TYPE type_;
};